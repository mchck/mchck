#include <usb/usb.h>
#include <usb/hid.h>


enum hid_ctrl_req_code {
	USB_CTRL_REQ_HID_GET_REPORT = 0x01,
	USB_CTRL_REQ_HID_GET_IDLE = 0x02,
	USB_CTRL_REQ_HID_GET_PROTOCOL = 0x03,
	USB_CTRL_REQ_HID_SET_REPORT = 0x09,
	USB_CTRL_REQ_HID_SET_IDLE = 0x0A,
	USB_CTRL_REQ_HID_SET_PROTOCOL = 0x0B
};

#define USB_FUNCTION_HID_IFACE_COUNT 1

static void
hid_set_report_done(void *buf, ssize_t len, void *cbdata)
{
	struct hid_ctx *ctx = cbdata;
	int ret = -1;

	ret = ctx->hidf->set_report(ctx->set_report_type, ctx->set_report_id, &buf, ctx->set_report_length);

	if (ret > 0)
		usb_handle_control_status(0);
	else
		usb_handle_control_status(1);
}

/*
 * Handle class (HID) specific calls.
 *
 * see hid_handle_control()
 */
static int
hid_handle_control_class(struct usb_ctrl_req_t *req, struct hid_ctx *ctx)
{
	size_t len = 0;

	/* XXX maintain state for all report descriptors */

	switch ((enum hid_ctrl_req_code)req->bRequest) {
	case USB_CTRL_REQ_HID_GET_REPORT: {
		enum hid_report_type report_type = req->wValue >> 8;
		uint8_t report_id = req->wValue & 0xff;
		int ret = -1;

		ctx->get_report_outstanding_length = req->wLength;
		if (ctx->hidf->get_report)
			ret = ctx->hidf->get_report(ctx, report_type, report_id);
		if (ret <= 0) {
			ctx->get_report_outstanding_length = 0;
			usb_handle_control_status(1);
		}
		return (1);
	}

	case USB_CTRL_REQ_HID_SET_REPORT: {
		int ret = -1;
		void *buf = NULL;
		ctx->set_report_length = req->wLength;
		ctx->set_report_type = req->wValue >> 8;
		ctx->set_report_id = req->wValue & 0xff;

		if (ctx->hidf->set_report)
			ret = ctx->hidf->set_report(ctx->set_report_type, ctx->set_report_id, &buf, ctx->set_report_length);
		if (ret > 0)
			usb_ep0_rx(buf, ctx->set_report_length, hid_set_report_done, ctx);
		else
			usb_handle_control_status(1);
		return (1);
	}

	case USB_CTRL_REQ_HID_GET_IDLE:
		/* XXX implement */
		usb_ep0_tx_cp(&len, 1, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);

	case USB_CTRL_REQ_HID_SET_IDLE:
		/* XXX implement */
		usb_handle_control_status(0);
		return (1);

	case USB_CTRL_REQ_HID_GET_PROTOCOL:
		/* XXX implement */
		/* usb_ep0_tx_cp(&len, 1, req->wLength, NULL, NULL); */
		/* usb_handle_control_status(0); */
		return (0);

	case USB_CTRL_REQ_HID_SET_PROTOCOL:
		/* XXX implement */
		usb_handle_control_status(0);
		return (1);

	default:
		return (0);
	}
}

/*
 * Handle non standard and non device calls.
 *
 * return non-zero if the call was handled.
 */
int
hid_handle_control(struct usb_ctrl_req_t *req, void *data)
{
	struct hid_ctx *ctx = data;
	const void *buf = NULL;
	size_t len = 0;

	if (req->type == USB_CTRL_REQ_CLASS)
		return (hid_handle_control_class(req, ctx));

	switch (req->bRequest) {
	case USB_CTRL_REQ_GET_DESCRIPTOR:
		if (req->wValueHigh == USB_HID_REPORT_DESC_TYPE_REPORT) {
			buf = ctx->hidf->report_desc;
			len = ctx->hidf->report_desc_size;
		} else {
			if (ctx->hidf->get_descriptor)
				len = ctx->hidf->get_descriptor(req->wValueHigh, req->wValueLow, &buf);
		}
		if (len == 0)
			return (0);
		usb_ep0_tx_cp(buf, len, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);

	case USB_CTRL_REQ_SET_DESCRIPTOR: // TODO
		return (0);

	default:
		return (0);
	}
}

void
hid_init(const struct usbd_function *f, int enable)
{
	const struct hid_function *hidf = (void *)f;
	struct hid_ctx *ctx = hidf->ctx;

	if (enable) {
		ctx->hidf = hidf;
		usb_attach_function(&hidf->usb_func, &ctx->header);
		if (hidf->report_max_size > 0)
			ctx->tx_pipe = usb_init_ep(&ctx->header, HID_TX_EP, USB_EP_TX, hidf->report_max_size);
	}
}

void
hid_update_data(struct hid_ctx *ctx, uint8_t report_id, const void *data, size_t len)
{
	if (ctx->get_report_outstanding_length != 0) {
		usb_ep0_tx(data, len, ctx->get_report_outstanding_length, NULL, NULL);
		ctx->get_report_outstanding_length = 0;
		usb_handle_control_status(0);
	} else if (ctx->hidf->report_max_size > 0)  {
		usb_tx(ctx->tx_pipe, data, len, ctx->hidf->report_max_size, NULL, NULL);
	}
}
