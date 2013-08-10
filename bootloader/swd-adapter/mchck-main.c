#include <mchck.h>

#include <usb/usb.h>
#include <usb/cdc-acm.h>

#include "swd.h"

#include "desc.c"

static struct cdc_ctx cdc;

/* Glue for SWD */

void
pin_configure(enum swd_pin pin, enum swd_pin_mode mode)
{
        gpio_mode(pin, mode);
}

void
pin_write(enum swd_pin pin, int val)
{
        gpio_write(pin, val);
}

int
pin_read(enum swd_pin pin)
{
        return (gpio_read(pin));
}

size_t
reply_space(void)
{
        return (cdc_write_space(&cdc));
}

void
reply_write(const uint8_t *buf, size_t len)
{
        cdc_write(buf, len, &cdc);
}

static void
new_data(uint8_t *data, size_t len)
{
        static uint8_t buf[10];
        static size_t buflen;

        onboard_led(-1);
        while (buflen + len > 0) {
                size_t copylen = sizeof(buf) - buflen;

                if (len < copylen)
                        copylen = len;
                memcpy(buf + buflen, data, copylen);
                len -= copylen;
                buflen += copylen;
                data += copylen;

                const uint8_t *p = process_buf(buf, buflen);
                if (p == buf)
                        break;
                buflen -= p - buf;
                memcpy(buf, p, buflen);
        }
        cdc_read_more(&cdc);
}


static void
init_cdc(int config)
{
        cdc_init(new_data, NULL, &cdc);
}

const static struct usbd_config cdc_config = {
        .init = init_cdc,
        .desc = &cdc_desc_config.config,
        .function = { &cdc_function, NULL },
};

const struct usbd_device cdc_device = {
        .dev_desc = &cdc_dev_desc,
        .string_descs = cdc_string_descs,
        .configs = { &cdc_config, NULL }
};

void
main(void)
{
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
