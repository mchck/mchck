#include <mchck.h>

#include <usb/usb.h>
#include <usb/cdc-acm.h>

static struct cdc_ctx cdc;

static int temp_count = 0;

static void
temp_done(uint16_t data, int error, void *cbdata)
{
        unsigned accum volt = adc_as_voltage(data);
        accum volt_diff = volt - 0.719k;
        accum temp_diff = volt_diff * (1000K / 1.715K);
        accum temp_deg = 25k - temp_diff;

        printf("raw: %u, temp: %.1k\r\n", data, temp_deg);
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
        cdc_set_stdout(&cdc);
}

static const struct usbd_device cdc_device =
        USB_INIT_DEVICE(0x2323,              /* vid */
                        3,                   /* pid */
                        u"mchck.org",        /* vendor */
                        u"temperature test", /* product" */
                        (init_vcdc,          /* init */
                         CDC)                /* functions */
                );

void
main(void)
{
        adc_init();
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
