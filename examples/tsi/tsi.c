#include <mchck.h>

#include <usb/usb.h>
#include <usb/cdc-acm.h>

static struct cdc_ctx cdc;

void
TSI0_Handler(void)
{
        if (TSI0.gencs.eosf == 1){
                TSI0.gencs.eosf = 1; /* clear end of scan flag */
                printf("raw: %u\r\n", TSI0.cntr[0].ctn1);
                onboard_led(ONBOARD_LED_TOGGLE);
        
        }
}

static void
new_data(uint8_t *data, size_t len)
{
        TSI0.gencs.swts = 1;
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
                        u"tsi test",         /* product" */
                        (init_vcdc,          /* init */
                         CDC)                /* functions */
                );

int
main(void)
{
    SIM.scgc5.tsi = 1; /* turn on clock to tsi */

    /* PTB0 is TSI0_CH0 */
    TSI0.pen.raw = ((struct TSI_PEN){
            .pen = 1,
            }).raw;
    TSI0.gencs.raw = ((struct TSI_GENCS){
            .stm = 0,
            .tsiie = 1,
            .esor = 1,
            .nscn = 15,
            .ps = TSI_OSC_PS_2,
            }).raw;

    int_enable(IRQ_TSI0);

    TSI0.gencs.tsien = 1;

    usb_init(&cdc_device);
    sys_yield_for_frogs();
}
