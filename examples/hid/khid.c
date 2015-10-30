#include <mchck.h>
#include <usb/usb.h>
#include <usb/hid.h>

#include "hid.desc.h"

static struct mouse_data mouse_data;
static struct timeout_ctx t;

/* XXX pass cb data */
int
get_report(struct hid_ctx *ctx, enum hid_report_type type, uint8_t report_id)
{
	if (type != USB_HID_REPORT_TYPE_INPUT)
		return (0);
	hid_update_data(ctx, report_id, &mouse_data, sizeof(mouse_data));
	return (1);
}

static void
mouse_timeout(void *cbdata)
{
	hid_update_data(&hid_ctx, 0, &mouse_data, sizeof(mouse_data));
	timeout_add(&t, 5, mouse_timeout, NULL);
}

void
init_my_hid(int config) // see desc.h
{
	mouse_timeout(NULL);
}

void
main(void)
{
	timeout_init();
	mouse_data.x = 1; // hehe
	usb_init(&hid_dev);
	sys_yield_for_frogs();
}
