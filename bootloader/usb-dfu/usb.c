#include <sys/types.h>

#include <inttypes.h>
#include <string.h>
#include <wchar.h>

#include "usb.h"


struct usbd_t usb;


/**
 * Returns: 0 when this is was the last transfer, 1 if there is still
 * more to go.
 */
/* Defaults to EP0 for now */
int
usb_tx_next(void)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.tx;

	/**
	 * Us being here means the previous transfer just completed
	 * successfully.  That means the host just toggled its data
	 * sync bit, and so do we.
	 */
	s->data01 ^= 1;

	if (s->transfer_size > 0) {
		size_t thislen = s->transfer_size;

		if (thislen > s->ep_maxsize)
			thislen = s->ep_maxsize;

		void *addr = s->data_buf + s->pos;
		s->pos += thislen;
		s->transfer_size -= thislen;

		usb_tx_queue_next(s, addr, thislen);
		s->pingpong ^= 1;

		return (1);
	}

	/**
	 * All data has been shipped.  Do we need to send a short
	 * packet?
	 */
	if (s->short_transfer) {
		s->short_transfer = 0;
		usb_tx_queue_next(s, NULL, 0);
		s->pingpong ^= 1;
		return (1);
	}

	if (s->callback)
		s->callback(s->data_buf, 0, s->callback_data);

	return (0);
}

/**
 * send USB data (IN device transaction)
 *
 * So far this function is specialized for EP 0 only.
 *
 * Returns: size to be transfered, or -1 on error.
 */
int
usb_tx(void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.tx;

	s->data_buf = buf;
	s->transfer_size = len;
	s->pos = 0;
	s->callback = cb;
	s->callback_data = cb_data;
	if (s->transfer_size > reqlen)
		s->transfer_size = reqlen;
	if (s->transfer_size < reqlen && s->transfer_size % s->ep_maxsize == 0)
		s->short_transfer = 1;
	else
		s->short_transfer = 0;

	/* usb_tx_next() flips the data toggle, so invert this here. */
	s->data01 ^= 1;
	usb_tx_next();
	return (s->transfer_size);
}

int
usb_tx_cp(void *buf, size_t len, size_t reqlen, ep_callback_t cb, void *cb_data)
{
	enum usb_ep_pingpong pp = usb.ep0_state.tx.pingpong;
	void *destbuf = usb.ep0_buf[pp];

	if (len > EP0_BUFSIZE)
		return (-1);
	memcpy(destbuf, buf, len);

	return (usb_tx(destbuf, len, len, NULL, NULL));
}


/**
 * Returns: 0 when this is was the last transfer, 1 if there is still
 * more to go.
 */
/* Defaults to EP0 for now */
/* XXX pass usb_stat to validate pingpong */
int
usb_rx_next(void)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.rx;

	/**
	 * Us being here means the previous transfer just completed
	 * successfully.  That means the host just toggled its data
	 * sync bit, and so do we.
	 */
	s->data01 ^= 1;

	size_t thislen = usb_ep_get_transfer_size(0, USB_EP_RX, s->pingpong);

	s->transfer_size -= thislen;
	s->pos += thislen;

	/**
	 * We're done with this buffer now.  Switch the pingpong now
	 * before we might have to receive the next piece of data.
	 */
	s->pingpong ^= 1;

	/**
	 * If this is a short transfer, or we received what we
	 * expected, we're done.
	 */
	if (thislen < s->ep_maxsize || s->transfer_size == 0) {
		if (s->callback)
			s->callback(s->data_buf, s->pos, s->callback_data);
		return (0);
	}

	/**
	 * Otherwise we still need to receive more data.
	 */
	size_t nextlen = s->transfer_size;

        if (nextlen > s->ep_maxsize)
                nextlen = s->ep_maxsize;

        void *addr = s->data_buf + s->pos;
	usb_rx_queue_next(s, addr, nextlen);

	return (1);
}

/**
 * Receive USB data (OUT device transaction)
 *
 * So far this function is specialized for EP 0 only.
 *
 * Returns: size to be received, or -1 on error.
 */
int
usb_rx(void *buf, size_t len, ep_callback_t cb, void *cb_data)
{
	struct usbd_ep_pipe_state_t *s = &usb.ep0_state.rx;

	s->data_buf = buf;
	s->transfer_size = len;
	s->pos = 0;
	s->callback = cb;
	s->callback_data = cb_data;

	size_t thislen = s->transfer_size;
	if (thislen > s->ep_maxsize)
		thislen = s->ep_maxsize;

	usb_rx_queue_next(s, s->data_buf, thislen);
	return (len);
}


static void usb_setup_control(void);

static void
usb_handle_control_done(void *data, ssize_t len, void *cbdata)
{
	usb.ctrl_state = USBD_CTRL_STATE_IDLE;
	if (usb.state == USBD_STATE_SETTING_ADDRESS) {
		usb.state = USBD_STATE_ADDRESS;
		usb_set_addr(usb.address);
	}
	usb_setup_control();
}

void
usb_handle_control_status(void *data, ssize_t len, void *cbdata)
{
	usb.ctrl_state = USBD_CTRL_STATE_STATUS;

	/* empty status transfer */
	switch (usb.ctrl_dir) {
	case USB_CTRL_REQ_IN:
		usb.ep0_state.rx.data01 = USB_DATA01_DATA1;
		usb_rx(NULL, 0, usb_handle_control_done, NULL);
		break;

	default:
		usb.ep0_state.tx.data01 = USB_DATA01_DATA1;
		usb_tx(NULL, 0, 1 /* short packet */, usb_handle_control_done, NULL);
		break;
	}
}

/**
 *
 * Great resource: http://wiki.osdev.org/Universal_Serial_Bus
 *
 * Control Transfers
 * -----------------
 *
 * A control transfer consists of a SETUP transaction (1), zero or
 * more data transactions (IN or OUT) (2), and a final status
 * transaction (3).
 *
 * Token sequence (data toggle):
 * 1.  SETUP (0)
 * (2a. OUT (1) ... (toggling))
 * 3a. IN (1)
 *
 * or
 * 1.  SETUP (0)
 * 2b. IN (1) ... (toggling)
 * 3b. OUT (1)
 *
 * Report errors by STALLing the control EP after (1) or (2), so that
 * (3) will STALL.  Seems we need to clear the STALL after that so
 * that the next SETUP can make it through.
 *
 *
 */

/**
 * The following code is not written defensively, but instead only
 * asserts values that are essential for correct execution.  It
 * accepts a superset of the protocol defined by the standard.  We do
 * this to save space.
 */

static void
usb_handle_control(void *data, ssize_t len, void *cbdata)
{
	struct usb_ctrl_req_t *req = data;
	uint16_t zero16 = 0;

	usb_clear_transfers();
	usb.ctrl_state = USBD_CTRL_STATE_DATA;
	usb.ctrl_dir = req->in;

	if (req->type != USB_CTRL_REQ_STD) {
		/* XXX pass on to higher levels */
		goto err;
	}

	/* Only STD requests here */
	switch (req->bRequest) {
	case USB_CTRL_REQ_GET_STATUS:
		/**
		 * Because we don't support remote wakeup or
		 * self-powered operation, and we are specialized to
		 * only EP 0 so far, all GET_STATUS replies are just
		 * empty.
		 */
		usb_tx_cp(&zero16, sizeof(zero16), req->wLength,
			  usb_handle_control_status, NULL);
		break;

	case USB_CTRL_REQ_CLEAR_FEATURE:
	case USB_CTRL_REQ_SET_FEATURE:
		/**
		 * Nothing to do.  Maybe return STALLs on illegal
		 * accesses?
		 */
		usb_handle_control_status(NULL, 0, NULL);
		break;

	case USB_CTRL_REQ_SET_ADDRESS:
		/**
		 * We must keep our previous address until the end of
		 * the status stage;  therefore we can't set the
		 * address right now.  Since this is a special case,
		 * the EP 0 handler will take care of this later on.
		 */
		usb.address = req->wValue & 0x7f;
		usb.state = USBD_STATE_SETTING_ADDRESS;
		usb_handle_control_status(NULL, 0, NULL);
		break;

	case USB_CTRL_REQ_GET_DESCRIPTOR: {
		struct usb_desc_generic_t *desc;

		switch (req->wValue >> 8) {
		case USB_DESC_DEV:
			desc = (void *)usb.dev_desc;
			break;
		case USB_DESC_CONFIG:
			desc = (void *)usb.config_desc;
			break;
		case USB_DESC_STRING:
			/* XXX */
		default:
			goto err;
		}
		usb_tx(desc, desc->bLength, req->wLength,
		       usb_handle_control_status, NULL);
		break;
	}

	case USB_CTRL_REQ_GET_CONFIGURATION:
		usb_tx_cp(&usb.config, 1, req->wLength,
			  usb_handle_control_status, NULL); /* XXX implicit LE */
		break;

	case USB_CTRL_REQ_SET_CONFIGURATION:
		/* XXX check config */
		usb.config = req->wValue;
		usb.state = USBD_STATE_CONFIGURED;
		usb_handle_control_status(NULL, 0, NULL);
		break;

	case USB_CTRL_REQ_GET_INTERFACE:
		/* We only support iface setting 0 */
		usb_tx_cp(&zero16, 1, req->wLength,
			  usb_handle_control_status, NULL);
		break;

	case USB_CTRL_REQ_SET_INTERFACE:
		/* We don't support alternate iface settings */
		goto err;

	default:
		goto err;
	}

	goto out;

err:
	usb_ep_stall(0);
	usb_setup_control();
out:
	usb_enable_xfers();
}

void
usb_setup_control(void)
{
	void *buf = usb.ep0_buf[usb.ep0_state.rx.pingpong];

	usb.ep0_state.rx.data01 = USB_DATA01_DATA0;
	usb.ep0_state.tx.data01 = USB_DATA01_DATA1;
	usb.ep0_state.rx.ep_maxsize = EP0_BUFSIZE;
	usb.ep0_state.tx.ep_maxsize = EP0_BUFSIZE;
	usb_rx(buf, EP0_BUFSIZE, usb_handle_control, NULL);
}

void
usb_handle_control_ep(struct usb_xfer_info *stat)
{
	switch (usb_get_xfer_pid(stat)) {
	case USB_PID_SETUP:
	case USB_PID_OUT:
		usb_rx_next();
		break;
	case USB_PID_IN:
		usb_tx_next();
		break;
	default:
		break;
	}
}

void
usb_start(struct usb_desc_dev_t *dev_desc, struct usb_desc_config_t *config_desc)
{
	usb.dev_desc = dev_desc;
	usb.config_desc = config_desc;
	usb_setup_control();
	usb_enable_xfers();
}

/**
 * This is called by the interrupt handler
 */
void
usb_handle_transaction(struct usb_xfer_info *info)
{
	/* XXX for now only control EP */
	usb_handle_control_ep(info);
}
