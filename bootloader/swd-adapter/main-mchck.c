#include <mchck.h>

#include <usb/usb.h>
#include <usb/cdc-acm.h>

#include "swduino/swd.h"

#include "desc.c"

static struct cdc_ctx cdc;

/* Glue for SWD */

void
signal_led(void)
{
        onboard_led(ONBOARD_LED_TOGGLE);
}

void
pin_configure(enum swd_pin pin, enum swd_pin_mode mode)
{
        gpio_dir(pin, mode);
        pin_mode(pin, PIN_MODE_SLEW_SLOW);
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

int
outpipe_space(size_t len)
{
        return (cdc_write_space(&cdc) >= len);
}

void
reply_write(const uint8_t *buf, size_t len)
{
        cdc_write(buf, len, &cdc);
}

/* end glue */


static int draining = 0;

static void
new_data(uint8_t *data, size_t len)
{
        draining = process_data(data, len);

        if (!draining)
                cdc_read_more(&cdc);
}

static void
space_available(size_t len)
{
        if (draining)
                new_data(NULL, 0);
}

static void
init_cdc(int config)
{
        cdc_init(new_data, space_available, &cdc);
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
