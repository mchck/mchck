#include <stdio.h>

#include <usb/usb.h>
#include <usb/cdc-acm.h>

#include "desc.c"

static struct cdc_ctx cdc;

static void
new_data(uint8_t *data, size_t len)
{
        printf("received: `%*s'\n", len, data);
        cdc_write(data, len, &cdc);
        cdc_read_more(&cdc);
}

static void
init_vcdc(int config)
{
        cdc_init(new_data, NULL, &cdc);
}


const static struct usbd_config cdc_config = {
        .init = init_vcdc,
        .desc = &cdc_desc_config.config,
        .function = { &cdc_function, NULL },
};

const struct usbd_device cdc_device = {
        .dev_desc = &cdc_dev_desc,
        .string_descs = cdc_string_descs,
        .configs = { &cdc_config, NULL }
};

int
main(void)
{
        usb_init(&cdc_device);
        vusb_main_loop();
}
