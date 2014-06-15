#include <mchck.h>
#include <usb/usb.h>
#include <usb/hid.h>

#include "hid.desc.h"

static struct feature_report feature_report;

/* XXX pass cb data */
int
get_report(struct hid_ctx *ctx, enum hid_report_type type, uint8_t report_id)
{
	if (type != USB_HID_REPORT_TYPE_FEATURE)
		return (0);
	hid_update_data(ctx, report_id, &feature_report, sizeof(feature_report));
	return (1);
}

int
set_report(enum hid_report_type type, uint8_t report_id, void **data, size_t len)
{
	if (type != USB_HID_REPORT_TYPE_FEATURE)
		return (0);
	if (*data == NULL)
		*data = &feature_report;
	else /* transfer done */
		onboard_led(ONBOARD_LED_TOGGLE);
	return (1);
}

void
init_my_hid(int config) // see desc.h
{
}

void
main(void)
{
	usb_init(&hid_dev);
	sys_yield_for_frogs();
}
