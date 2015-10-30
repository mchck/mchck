/* -*- mode: c -*- */

#include <mchck.h>

#include <usb/usb.h>
#include <usb/dfu.h>
#include <usb/cdc-acm.h>


#ifdef DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif


/**
 * 1. set pcs polarity to low
 * 2. configure pins in port register
 * 2. enable power for target
 * 3. switch back pcs polarity
 * 4. spi transaction
 * 5. disable power
 *
 * Pin Mapping:
 * D0: alt2, SPI0_PCS0 -> /EZ_CS, PTA4, pin 21
 * D1: alt2, SPI0_SCK  -> EZ_CLK, PTA0, pin 17
 * D2: alt2, SPI0_SOUT -> EZ_DI,  PTA1, pin 18
 * D3: alt2, SPI0_SIN  <- EZ_DO,  PTA2, pin 19
 * D4: alt1, GPIO      -> float = power off; drive 0 = power on
 * D5: alt1, GPIO      <- button, pull-up, filter
 * D6: alt1, GPIO      <- target reset, pin 26
 * D7: alt1, GPIO      <- target onboard LED, PTB16, pin 31
 * C7: alt1, GPIO      -> green LED
 * C5: alt1, GPIO      -> red LED
 */

enum {
        EZPORT_CS    = PIN_PTD0,
        EZPORT_CLK   = PIN_PTD1,
        EZPORT_DI    = PIN_PTD2,
        EZPORT_DO    = PIN_PTD3,
        EZPORT_POWER = PIN_PTD4,
        PROG_BUTTON  = PIN_PTD5,
        TARGET_RESET = PIN_PTD6,
        TARGET_LED   = PIN_PTD7,
        LED_SUCCESS  = PIN_PTC7,
        LED_FAIL     = PIN_PTC5,
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
        } flexram      : 1;
        uint8_t _rsvd0 : 2;
        uint8_t wef    : 1;
        uint8_t fs     : 1;
        UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct EZPORT_STATUS, 8);

enum state_event {
        ev_button = 1,
        ev_reset = 2,
        ev_led = 3,
        ev_cmd_done = 4,
        ev_timeout = 5,
};

enum result_status {
        RESULT_UNKNOWN,
        RESULT_SUCCESS,
        RESULT_FAIL
};

extern uint8_t _binary_payload_bin_start[];
extern int _binary_payload_bin_size;


static struct cdc_ctx cdc;
static struct EZPORT_STATUS ezport_status;

%% machine ezport;
%% import "ez-port-flash.rl";
%% write data;


static void statemachine(enum state_event ev);


static void
walk_state(void *cbdata)
{
        struct EZPORT_STATUS *status = cbdata;

        ezport_status = *status;
        debug_printf("  status: %#.2x\r\n", ezport_status.raw);
        statemachine((uintptr_t)ev_cmd_done);
}

static void
timeout(void *data)
{
        debug_printf("TIMEOUT\r\n");
        statemachine(ev_timeout);
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
signal_leds(enum result_status status)
{
        gpio_write(LED_SUCCESS, 0);
        gpio_write(LED_FAIL, 0);

        switch (status) {
        case RESULT_SUCCESS:
                gpio_write(LED_SUCCESS, 1);
                break;
        case RESULT_FAIL:
                gpio_write(LED_FAIL, 1);
                break;
        default:
                break;
        }
}

%%{
action init_vars {
        program_address = 0;
}

action signal_leds_off {
        signal_leds(RESULT_UNKNOWN);
}

action signal_leds_success {
        signal_leds(RESULT_SUCCESS);
}

action signal_leds_fail {
        signal_leds(RESULT_FAIL);
}

action disable_timeout {
        timeout_cancel(&t);
}

action enable_power {
        /* pull CS low */
        gpio_write(EZPORT_CS, 0);
        gpio_dir(EZPORT_CS, GPIO_OUTPUT);

        /* enable power */
        gpio_write(EZPORT_POWER, 0);
        gpio_dir(EZPORT_POWER, GPIO_OUTPUT);

        debug_printf("power\r\n");
        timeout_add(&t, 10, timeout, NULL);
}

action wait_powerup {
        timeout_add(&t, 50, timeout, NULL);
}

action enable_spi {
        spi_init();
        pin_mode(EZPORT_CS, PIN_MODE_MUX_ALT2);
        pin_mode(EZPORT_CLK, PIN_MODE_MUX_ALT2);
        pin_mode(EZPORT_DI, PIN_MODE_MUX_ALT2);
        pin_mode(EZPORT_DO, PIN_MODE_MUX_ALT2);
}

action check_status {
        static struct spi_ctx rdsr_ctx;
        static const enum EZPORT_CMD rdsr_cmd = EZPORT_RDSR;
        static uint8_t rxbuf[2];

        spi_queue_xfer(&rdsr_ctx, EZPORT_SPI_CS,
                       &rdsr_cmd, 1, rxbuf, 2,
                       walk_state, &rxbuf[1]);
}

action not_write_protected {
        !(ezport_status.fs && ezport_status.bedis)
}

action write_busy {
        ezport_status.wip
}

action bulk_erase {
        static struct spi_ctx be_ctx;
        static const enum EZPORT_CMD be_cmd = EZPORT_BE;

        write_enable();
        spi_queue_xfer(&be_ctx, EZPORT_SPI_CS,
                       &be_cmd, 1, NULL, 0,
                       NULL, NULL);

        debug_printf("erase\r\n");
        /* Datasheet 6.4.1.2 */
        timeout_add(&t, 300, timeout, NULL);
}

action program_data_left {
        program_address < (size_t)&_binary_payload_bin_size
}

action program_sector {
        static struct spi_ctx_bare sp_ctx;
        static struct sg tx_sg[2];
        static uint8_t header[4];
        size_t len = FLASH_SECTOR_SIZE;

        write_enable();
        header[0] = EZPORT_SP;
        header[1] = program_address >> 16;
        header[2] = program_address >> 8;
        header[3] = program_address;
        sg_init(tx_sg,
                (void *)header, 4,
                _binary_payload_bin_start + program_address, len);
        program_address += len;

        spi_queue_xfer_sg(&sp_ctx, EZPORT_SPI_CS,
                          tx_sg, NULL,
                          NULL, NULL);

        debug_printf("program %d\r\n", program_address - len);
        /* Datasheet 6.4.1.2 */
        timeout_add(&t, 200, timeout, NULL);
}

action reset_target {
        static struct spi_ctx reset_ctx;
        static const enum EZPORT_CMD reset_cmd = EZPORT_RESET;

        spi_queue_xfer(&reset_ctx, EZPORT_SPI_CS,
                       &reset_cmd, 1, NULL, 0,
                       NULL, NULL);
        debug_printf("reset\r\n");
        timeout_add(&t, 1000, timeout, NULL);
}

action disable_power {
        pin_mode(EZPORT_CS, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_CLK, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_DI, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_DO, PIN_MODE_MUX_ANALOG);
        pin_mode(EZPORT_POWER, PIN_MODE_MUX_ANALOG);
        debug_printf("power off\r\n");
}
}%%

%%{
        action restart {
                fgoto main;
        }

main := (

        start: (
                ev_button @init_vars @signal_leds_off @enable_power -> starting1
                ),
        starting1: (
                ev_reset >disable_timeout @wait_powerup -> starting2
                ),
        starting2: (
                ev_reset -> starting2 |
                ev_timeout @enable_spi @check_status -> ezport_running
        ),
        ezport_running: (
                ev_cmd_done when not_write_protected >disable_timeout @bulk_erase @check_status -> erasing
                ),
        erasing: (
                ev_cmd_done when write_busy @2 @check_status -> erasing |
                ev_cmd_done @0 >disable_timeout @program_sector @check_status -> programming
                ),
        programming: (
                ev_cmd_done when write_busy @2 @check_status -> programming |
                ev_cmd_done when program_data_left @1 >disable_timeout @program_sector @check_status -> programming |
                ev_cmd_done @0 >disable_timeout @reset_target -> app_running
                ),
        app_running: (
                ev_led >disable_timeout @disable_power @signal_leds_success -> final
                )
        )*

        $err(disable_timeout) $err(disable_power) $err(signal_leds_fail) $err(restart);
}%%

static void
statemachine(enum state_event ev)
{
        /* state vars */
        static size_t program_address;
        static struct timeout_ctx t;
        static int cs = %%{ write start; }%%;

        /* execution vars */
        const static enum state_event *eof = NULL;
        const enum state_event *p = &ev, * const pe = p + 1;

        debug_printf("  event: %d\r\n", ev);

        %% write exec;
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
        if (pin_physport_from_pin(TARGET_LED)->pcr[pin_physpin_from_pin(TARGET_LED)].isf) {
                pin_physport_from_pin(TARGET_LED)->pcr[pin_physpin_from_pin(TARGET_LED)].raw |= 0; /* clear isf */
                statemachine(ev_led);
        }
}

static void
init(void)
{
        /* set modes before to enable the port */
        gpio_dir(PROG_BUTTON, GPIO_INPUT);
        pin_mode(PROG_BUTTON, PIN_MODE_PULLUP);
        gpio_dir(TARGET_RESET, GPIO_INPUT);
        gpio_dir(TARGET_LED, GPIO_INPUT);

        /* set digital debounce/filter */
        pin_physport_from_pin(PROG_BUTTON)->dfcr.cs = PORT_CS_LPO;
        pin_physport_from_pin(PROG_BUTTON)->dfwr.filt = 31;

        /* button interrupt */
        pin_physport_from_pin(PROG_BUTTON)->dfer |= 1 << pin_physpin_from_pin(PROG_BUTTON);
        pin_physport_from_pin(PROG_BUTTON)->pcr[pin_physpin_from_pin(PROG_BUTTON)].irqc = PCR_IRQC_INT_FALLING;

        /* reset interrupt */
        pin_physport_from_pin(TARGET_RESET)->pcr[pin_physpin_from_pin(TARGET_RESET)].irqc = PCR_IRQC_INT_RISING;

        /* LED interrupt */
        pin_mode(TARGET_LED, PIN_MODE_PULLDOWN);
        pin_physport_from_pin(TARGET_LED)->pcr[pin_physpin_from_pin(TARGET_LED)].irqc = PCR_IRQC_INT_RISING;

        int_enable(IRQ_PORTD);

        gpio_dir(LED_SUCCESS, GPIO_OUTPUT);
        pin_mode(LED_SUCCESS, PIN_MODE_DRIVE_HIGH);
        gpio_dir(LED_FAIL, GPIO_OUTPUT);
        pin_mode(LED_FAIL, PIN_MODE_DRIVE_HIGH);

        timeout_init();
}

static void
init_ezport(int config)
{
        cdc_init(NULL, NULL, &cdc);
        cdc_set_stdout(&cdc);
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
