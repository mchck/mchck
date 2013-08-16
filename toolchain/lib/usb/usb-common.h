#ifndef _USB_COMMON_H
#define _USB_COMMON_H

/**
 * USB request data structures.
 */

enum usb_tok_pid {
	USB_PID_TIMEOUT = 0,
	USB_PID_OUT = 1,
	USB_PID_ACK = 2,
	USB_PID_DATA0 = 3,
	USB_PID_IN = 9,
	USB_PID_NAK = 10,
	USB_PID_DATA1 = 11,
	USB_PID_SETUP = 13,
	USB_PID_STALL = 14,
	USB_PID_DATAERR = 15
};

enum usb_ep_pingpong {
	USB_EP_PINGPONG_EVEN = 0,
	USB_EP_PINGPONG_ODD = 1
};

enum usb_ep_dir {
	USB_EP_RX = 0,
	USB_EP_TX = 1
};

enum usb_data01 {
	USB_DATA01_DATA0 = 0,
	USB_DATA01_DATA1 = 1
};

enum {
	EP0_BUFSIZE = 64
};

#endif
