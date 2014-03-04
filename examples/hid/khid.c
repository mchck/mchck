#include <mchck.h>
#include <usb/usb.h>
#include <usb/hid.h>

static void init_my_hid(int config);

#include "desc.h"

static struct hid_ctx hid_ctx;
static uint8_t idle_duration = 0;

static size_t
hid_get_descriptor(enum hid_report_descriptor_type type, uint8_t index, void **data_out)
{
	if (type != USB_HID_REPORT_DESC_TYPE_REPORT)
		return (0);
	*data_out = report_desc; // see desc.h
	return (REPORT_DESC_SIZE);
}

static size_t
hid_get_report(enum hid_report_type type, uint8_t report_id, void **data_out)
{
	if (type != USB_HID_REPORT_TYPE_INPUT)
		return (0);
	*data_out = &mouse_data;
	return (sizeof(struct mouse_data_t));
}

static void
hid_set_idle(const uint8_t duration, const uint8_t report_id)
{
	idle_duration = duration;
}

static struct hid_user_functions_t hid_funcs = {
	.get_descriptor = hid_get_descriptor,
	.get_report = hid_get_report,
	.set_idle = hid_set_idle,
};

static void
hid_send_data_cb(void *buf, ssize_t len)
{
	hid_send_data(&hid_ctx, &mouse_data, sizeof(struct mouse_data_t), MOUSE_TX_SIZE, hid_send_data_cb);
}

static void
init_my_hid(int config) // see desc.h
{
	hid_init(&hid_funcs, &hid_ctx);
	hid_send_data(&hid_ctx, &mouse_data, sizeof(struct mouse_data_t), MOUSE_TX_SIZE, hid_send_data_cb);
}

void
main(void)
{
	mouse_data.x = 1; // hehe
	usb_init(&hid_dev);
	sys_yield_for_frogs();
}
