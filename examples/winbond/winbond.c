#include <mchck.h>
#include <usb/usb.h>
#include <usb/dfu.h>
#include <usb/cdc-acm.h>

// W25Q128FV - 17 40 18 - 128Mbit
// W25Q64CV  - 16 40 17 - 64Mbit
// W25Q32DW  - 15 60 16 - 32Mbit
// W25Q16DW  - 14 60 15 - 16Mbit
// W25Q80BV  - 13 40 14 - 8Mbit

#define WREN            0x06    // Write Enable
#define READ_SR1        0x05    // Read Status Register-1
#define PAGE_PROGRAM    0x02    // Page Program
#define SECTOR_ERASE    0x20    // Sector Erase
#define CHIP_ERASE      0xC7    // Chip Erase

#define tb(a)   (((a) >> 16) & 0xFF)
#define mb(a)   (((a) >> 8) & 0xFF)
#define lb(a)   ((a) & 0xFF)

void delay(int n)
{
    while (--n)
        asm("nop");
}

void cb_debug(void *message)
{
    printf("%s:\n", (char *)message);
}

void cb_status(void *data)
{
    uint8_t *reply = data;
    printf("status: ");
    for (int i = 0; i < 16; i++) {
        printf("%x ", reply[i]);
    }
    printf("\n");
}

void send_status() {
    static struct spi_ctx ctx;
    static const uint8_t cmd[] = {READ_SR1};
    static uint8_t reply[16];

    spi_queue_xfer(&ctx, SPI_PCS4,
                   cmd, sizeof(cmd), reply, sizeof(reply),
                   cb_status, reply);
}

void cb_read(void *data)
{
    uint8_t *reply = data;
    printf("read:  ");
    for (int i = 0; i < 16; i++) {
        printf("%x ", reply[i]);
    }
    printf("\n");
}

void send_read(unsigned long a) {
    static struct spi_ctx ctx;
    static uint8_t cmd[4] = {0x03};
    cmd[1] = tb(a);
    cmd[2] = mb(a);
    cmd[3] = lb(a);
    static uint8_t reply[16];

    spi_queue_xfer(&ctx, SPI_PCS4,
                   cmd, sizeof(cmd), reply, sizeof(reply),
                   cb_read, reply);
}

void send_write_enable() {
    static struct spi_ctx ctx;
    static uint8_t cmd[] = {WREN};

    spi_queue_xfer(&ctx, SPI_PCS4,
                   cmd, sizeof(cmd), NULL, 0,
                   cb_debug, "write_enable");
}

void send_write(unsigned long a) {
    static struct spi_ctx ctx;
    static uint8_t cmd[] = {PAGE_PROGRAM,0,0,0,'d','e','a','d','b','e','e','f'};
    cmd[1] = tb(a);
    cmd[2] = mb(a);
    cmd[3] = lb(a);

    spi_queue_xfer(&ctx, SPI_PCS4,
                   cmd, sizeof(cmd), NULL, 0,
                   cb_debug, "write");
}

void send_page_erase(unsigned long a) {
    static struct spi_ctx ctx;
    static uint8_t cmd[] = {SECTOR_ERASE,0,0,0};
    a &= ~0xFFF;
    cmd[1] = tb(a);
    cmd[2] = mb(a);
    cmd[3] = lb(a);

    spi_queue_xfer(&ctx, SPI_PCS4,
                   cmd, sizeof(cmd), NULL, 0,
                   cb_debug, "erase");
}

void send_erase() {
    static struct spi_ctx ctx;
    static const uint8_t cmd[] = {CHIP_ERASE};

    spi_queue_xfer(&ctx, SPI_PCS4,
                   cmd, sizeof(cmd), NULL, 0,
                   cb_debug, "erase");
}

static struct cdc_ctx cdc;

static void
new_data(uint8_t *data, size_t len)
{
    unsigned long a = 0x010203;

    onboard_led(ONBOARD_LED_TOGGLE);
    delay(100);
    for (int i = 0; i < len; i++) {
        if (data[i] == 's') {
            send_status();
        }
        if (data[i] == 'r') {
            send_read(a);
        }
        if (data[i] == 'w') {
            send_write_enable();
            send_write(a);
            send_status();
        }
        if (data[i] == 'p') {
            send_write_enable();
            send_page_erase(a);
            send_status();
        }
        if (data[i] == 'e') {
            send_write_enable();
            send_erase();
            send_status();
        }
    }
    onboard_led(ONBOARD_LED_TOGGLE);

    cdc_read_more(&cdc);
}

volatile int usb_ready = 0;

static void
init_winbond(int config)
{
    cdc_init(new_data, NULL, &cdc);
    cdc_set_stdout(&cdc);
    usb_ready = 1;
}

static const struct usbd_device cdc_device =
        USB_INIT_DEVICE(0x2323,                 /* vid */
                        3,                      /* pid */
                        u"mchck.org",           /* vendor */
                        u"Winbond test",        /* product" */
                        (init_winbond,          /* init */
                         CDC)                   /* functions */
                );

// NOTE: Making PTC0 an output and setting it HIGH causes the SPI interface
// to make it low, high and then low again at some time in the future. This
// defeats the purpose.
void
main(void)
{
    usb_init(&cdc_device);
    while (!usb_ready)
        ;

    onboard_led(ONBOARD_LED_TOGGLE);
    delay(100);

    gpio_dir(PIN_PTC0, GPIO_OUTPUT);
    gpio_write(PIN_PTC0, 1);

    spi_init();

    pin_mode(PIN_PTC0, PIN_MODE_MUX_ALT2);  // CS
    pin_mode(PIN_PTC5, PIN_MODE_MUX_ALT2);  // SCK
    pin_mode(PIN_PTC7, PIN_MODE_MUX_ALT2);  // MISO
    pin_mode(PIN_PTC6, PIN_MODE_MUX_ALT2);  // MOSI

    onboard_led(ONBOARD_LED_TOGGLE);

    sys_yield_for_frogs();
}
