#include <stdio.h>
#include <mchck.h>

#include "usb-serial-loopback.desc.h"


static struct cdc_ctx cdc;

static void
new_data(uint8_t *data, size_t len)
{
        printf("received: `%*s'\n", len, data);
        cdc_write(data, len, &cdc);
        cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
        cdc_init(new_data, NULL, &cdc);
}

int
main(void)
{
        usb_init(&cdc_device);
        vusb_main_loop();
}
