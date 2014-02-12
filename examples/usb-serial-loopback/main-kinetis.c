#include <mchck.h>

#include "usb-serial-loopback.desc.h"

static struct cdc_ctx cdc;

static void
new_data(uint8_t *data, size_t len)
{
        onboard_led(-1);
        cdc_write(data, len, &cdc);
        cdc_read_more(&cdc);
}

void
init_vcdc(int enable)
{
        if (enable) {
                cdc_init(new_data, NULL, &cdc);
        }
}

void
main(void)
{
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
