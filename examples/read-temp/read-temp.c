#include <mchck.h>

#include <usb/usb.h>
#include <usb/cdc-acm.h>

#include "desc.c"

static struct cdc_ctx cdc;

static int temp_count = 0;

static void
temp_done(uint16_t data, int error, void *cbdata)
{
        unsigned accum volt = adc_as_voltage(data);
        accum volt_diff = volt - 0.719k;
        accum temp_diff = volt_diff * (1000K / 1.715K);
        accum temp_deg = 25k - temp_diff;

        cdc_write((void *)&data, sizeof(data), &cdc);
        cdc_write((void *)&temp_deg, sizeof(temp_deg), &cdc);
        onboard_led(ONBOARD_LED_TOGGLE);

        if (--temp_count > 0)
                adc_sample_start(ADC_TEMP, temp_done, NULL);
}

static void
new_data(uint8_t *data, size_t len)
{
        int new_count = 0;

        for (; len > 0; ++data, --len) {
                switch (data[0]) {
                case '\r':
                        if (len > 1 && data[1] == '\n')
                                ++data, --len;
                        /* FALLTHROUGH */
                case '\n':
                        ++new_count;
                        break;
                }
        }

        if (temp_count == 0 && new_count > 0) {
                temp_count = new_count;
                adc_sample_start(ADC_TEMP, temp_done, NULL);
        } else {
                temp_count += new_count;
        }

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

void
main(void)
{
        adc_init();
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
