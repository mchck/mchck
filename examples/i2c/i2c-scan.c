#include <mchck.h>
#include "i2c-scan.desc.h"

#define TIMEOUT_REPEAT  3000

static struct cdc_ctx cdc;
static struct timeout_ctx t;

volatile uint8_t address;

void
part1(void *cbdata);

void
part2(enum i2c_result result, uint8_t *data, size_t length, void *cbdata)
{
        if (address == 0) {
                printf("\r\n  0123456789ABCDEF\r\n");
        }
        if (address % 16 == 0) {
                printf("%x:", address / 16);
        }
        printf("%d", result);
        address++;
        // Constrain the address to be between 0 and 128, 7 bits
        address &= 0x7f;
        if (address % 16 == 0) {
                printf("\r\n");
        }
        uint32_t delay = address == 0 ? TIMEOUT_REPEAT : 0;
        timeout_add(&t, delay, part1, NULL);
}

void
part1(void *cbdata)
{
        i2c_send(address, NULL, 0, I2C_STOP, part2, NULL);
}

static void
new_data(uint8_t *data, size_t len)
{
        cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
        cdc_init(new_data, NULL, &cdc);
        cdc_set_stdout(&cdc);
        address = 0;
        timeout_add(&t, 10, part1, NULL);
}

void
main(void)
{
        timeout_init();
        i2c_init(I2C_RATE_100);
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
