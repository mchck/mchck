#include <stdio.h>
#include <usb/usb.h>
#include <usb/hid.h>

#include "hid.desc.h"

struct mouse_data mouse_data = {
	.x = 1
};

static size_t
hid_get_report(enum hid_report_type type, uint8_t report_id, void **data_out)
{
	if (type != USB_HID_REPORT_TYPE_INPUT)
		return (0);
	*data_out = &mouse_data;
	return (sizeof(mouse_data));
}

static void
hid_set_idle(const uint8_t duration, const uint8_t report_id)
{
	printf("@@@@@ IDLE SET TO %d\n", duration * 4);
}

static void
hid_send_data_cb(void *buf, ssize_t len, void *extra)
{
	hid_send_data(&hid_ctx, &mouse_data, sizeof(mouse_data), sizeof(mouse_data), hid_send_data_cb, NULL);
}

void
init_my_hid(int config)
{
	hid_send_data(&hid_ctx, &mouse_data, sizeof(mouse_data), sizeof(mouse_data), hid_send_data_cb, NULL);
}

int
main(void)
{
	usb_init(&hid_dev);
	vusb_main_loop();
}
