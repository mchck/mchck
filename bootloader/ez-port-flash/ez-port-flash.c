#include <mchck.h>

#include <usb/usb.h>
#include <usb/dfu.h>
#include <usb/cdc-acm.h>

/**
 * 1. set pcs polarity to low
 * 2. configure pins in port register
 * 2. enable power for target
 * 3. switch back pcs polarity
 * 4. spi transaction
 * 5. disable power
 *
 * Pin Mapping:
 * D0: alt2, SPI0_PCS0 -> /EZ_CS
 * D1: alt2, SPI0_SCK  -> EZ_CLK
 * D2: alt2, SPI0_SOUT -> EZ_DI
 * D3: alt2, SPI0_SIN  <- EZ_DO
 * D4: alt1, GPIO      -> float = power off; drive 0 = power on
 * D5: alt1, GPIO      <- button, pull-up, filter
 * D6: alt1, GPIO      <- target reset
 */

enum {
        EZPORT_CS    = PIN_PTD0,
        EZPORT_CLK   = PIN_PTD1,
        EZPORT_DI    = PIN_PTD2,
        EZPORT_DO    = PIN_PTD3,
        EZPORT_POWER = PIN_PTD4,
        PROG_BUTTON  = PIN_PTD5,
        TARGET_RESET = PIN_PTD6,
};

enum {
        EZPORT_SPI_CS = SPI_PCS0
};

enum EZPORT_CMD {
        EZPORT_WREN           = 0x06,
        EZPORT_WRDI           = 0x04,
        EZPORT_RDSR           = 0x05,
        EZPORT_READ           = 0x03,
        EZPORT_FAST_READ      = 0x0b,
        EZPORT_SP             = 0x02,
        EZPORT_SE             = 0xd8,
        EZPORT_BE             = 0xc7,
        EZPORT_RESET          = 0xb9,
        EZPORT_WRFCCOB        = 0xba,
        EZPORT_FAST_RDFCCOB   = 0xbb,
        EZPORT_WRFLEXRAM      = 0xbc,
        EZPORT_RDFLEXRAM      = 0xbd,
        EZPORT_FAST_RDFLEXRAM = 0xbe,
};

struct EZPORT_STATUS {
        UNION_STRUCT_START(8);
        uint8_t wip    : 1;
        uint8_t wen    : 1;
        uint8_t bedis  : 1;
        enum {
                EZPORT_FLEXRAM_RAM    = 0,
                EZPORT_FLEXRAM_EEPROM = 1
        } flexram       : 1;
        uint8_t _rsvd0 : 2;
        uint8_t wef    : 1;
        uint8_t fs     : 1;
        UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct EZPORT_STATUS, 8);

enum state_event {
        ev_button,
        ev_reset,
        ev_cmd_done,
};


extern uint8_t _binary_payload_bin_start[];
extern int _binary_payload_bin_size;


static struct cdc_ctx cdc;
static struct EZPORT_STATUS ezport_status;

static void statemachine(enum state_event ev);


static void
enable_power(void)
{
        /* pull CS low */
        gpio_write(EZPORT_CS, 0);
        gpio_dir(EZPORT_CS, GPIO_OUTPUT);

        /* enable power */
        gpio_write(EZPORT_POWER, 0);
        gpio_dir(EZPORT_POWER, GPIO_OUTPUT);
}

static void
walk_state(void *cbdata)
{
        struct EZPORT_STATUS *status = cbdata;

        ezport_status = *status;
        statemachine((uintptr_t)ev_cmd_done);
}

static void
enable_spi(void)
{
        spi_init();
        pin_mode(EZPORT_CS, PIN_MODE_MUX_ALT2);
        pin_mode(EZPORT_CLK, PIN_MODE_MUX_ALT2);
        pin_mode(EZPORT_DI, PIN_MODE_MUX_ALT2);
        pin_mode(EZPORT_DO, PIN_MODE_MUX_ALT2);
}

static void
check_status(void)
{
        static struct spi_ctx rdsr_ctx;
        static const enum EZPORT_CMD rdsr_cmd = EZPORT_RDSR;
        static uint8_t rxbuf[2];

        spi_queue_xfer(&rdsr_ctx, EZPORT_SPI_CS,
                       &rdsr_cmd, 1, rxbuf, 2,
                       walk_state, &rxbuf[1]);
}

static void
write_enable(void)
{
        static struct spi_ctx wren_ctx;
        static const enum EZPORT_CMD wren_cmd = EZPORT_WREN;

        spi_queue_xfer(&wren_ctx, EZPORT_SPI_CS,
                       &wren_cmd, 1, NULL, 0,
                       NULL, NULL);
}

static void
bulk_erase(void)
{
        static struct spi_ctx be_ctx;
        static const enum EZPORT_CMD be_cmd = EZPORT_BE;

        write_enable();
        spi_queue_xfer(&be_ctx, EZPORT_SPI_CS,
                       &be_cmd, 1, NULL, 0,
                       NULL, NULL);
        check_status();
}

static size_t
program_sector(size_t addr)
{
        static struct spi_ctx_bare sp_ctx;
        static struct sg tx_sg[2];
        static uint8_t header[4];
        size_t len = FLASH_SECTOR_SIZE;

        write_enable();
        header[0] = EZPORT_SP;
        header[1] = addr >> 16;
        header[2] = addr >> 8;
        header[3] = addr;
        sg_init(tx_sg,
                (void *)header, 4,
                _binary_payload_bin_start + addr, len);
        spi_queue_xfer_sg(&sp_ctx, EZPORT_SPI_CS,
                          tx_sg, NULL,
                          NULL, NULL);
        check_status();

        return (addr + len);
}

static void
reset_target(void)
{
        static struct spi_ctx reset_ctx;
        static const enum EZPORT_CMD reset_cmd = EZPORT_RESET;

        spi_queue_xfer(&reset_ctx, EZPORT_SPI_CS,
                       &reset_cmd, 1, NULL, 0,
                       NULL, NULL);
}

static void
disable_power(void)
{
        pin_mode(EZPORT_CS, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_CLK, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_DI, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_DO, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_POWER, PIN_MODE_MUX_ANALOG);
}

static void
statemachine(enum state_event ev)
{
        static enum {
                st_off,
                st_power,
                st_ezport_running,
                st_erasing,
                st_programming,
                st_resetting,
                st_app_running,
        } state = st_off;
        static size_t program_address = 0;

        if (state == st_off && ev == ev_button) {
                state = st_power;

                enable_power();
        } else if (state == st_power && ev == ev_reset) {
                state = st_ezport_running;

                enable_spi();
                check_status();
        } else if (state == st_ezport_running && ev == ev_cmd_done) {
                /* Is chip bricked? */
                if (ezport_status.fs && ezport_status.bedis)
                        goto error;
                state = st_erasing;

                bulk_erase();
        } else if (state == st_erasing && ev == ev_cmd_done) {
                /* if still running, check again */
                if (ezport_status.wip) {
                        check_status();
                        return;
                }

                state = st_programming;
                goto program;
        } else if (state == st_programming && ev == ev_cmd_done) {
                /* if still running, check again */
                if (ezport_status.wip) {
                        check_status();
                        return;
                }

                /* repeat if not done */
                if (program_address < (size_t)&_binary_payload_bin_size) {
program:
                        program_address = program_sector(program_address);
                        return;
                }

                state = st_resetting;
                reset_target();
        } else if (state == st_resetting && ev == ev_reset) {
                state = st_app_running;
                /* count blinks, then power off */
        } else {
error:
                /* invalid transition */
                /* XXX show error */
                state = st_off;
                disable_power();
        }
}

void
PORTD_Handler(void)
{
        if (pin_physport_from_pin(PROG_BUTTON)->pcr[pin_physpin_from_pin(PROG_BUTTON)].isf) {
                pin_physport_from_pin(PROG_BUTTON)->pcr[pin_physpin_from_pin(PROG_BUTTON)].raw |= 0; /* clear isf */
                statemachine(ev_button);
        }
        if (pin_physport_from_pin(TARGET_RESET)->pcr[pin_physpin_from_pin(TARGET_RESET)].isf) {
                pin_physport_from_pin(TARGET_RESET)->pcr[pin_physpin_from_pin(TARGET_RESET)].raw |= 0; /* clear isf */
                statemachine(ev_reset);
        }
}

static void
init(void)
{
        /* set modes before to enable the port */
        gpio_dir(PROG_BUTTON, GPIO_INPUT);
        pin_mode(PROG_BUTTON, PIN_MODE_PULLUP);
        gpio_dir(TARGET_RESET, GPIO_INPUT);

        /* set digital debounce/filter */
        pin_physport_from_pin(PROG_BUTTON)->dfcr.cs = PORT_CS_LPO;
        pin_physport_from_pin(PROG_BUTTON)->dfwr.filt = 31;

        /* button interrupt */
        pin_physport_from_pin(PROG_BUTTON)->dfer |= 1 << pin_physpin_from_pin(PROG_BUTTON);
        pin_physport_from_pin(PROG_BUTTON)->pcr[pin_physpin_from_pin(PROG_BUTTON)].irqc = PCR_IRQC_INT_FALLING;

        /* reset interrupt */
        pin_physport_from_pin(TARGET_RESET)->pcr[pin_physpin_from_pin(TARGET_RESET)].irqc = PCR_IRQC_INT_RISING;

        int_enable(IRQ_PORTD);
}

static void
init_ezport(int config)
{
        cdc_init(NULL, NULL, &cdc);
}

static const struct usbd_device cdc_device =
        USB_INIT_DEVICE(0x2323,                 /* vid */
                        5,                      /* pid */
                        u"mchck.org",           /* vendor */
                        u"EZ-Port SPI flasher", /* product" */
                        (init_ezport,           /* init */
                         CDC)                   /* functions */
                );

void
main(void)
{
        init();
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
