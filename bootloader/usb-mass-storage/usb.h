#ifndef USB_H
#define USB_H

#include <sys/types.h>
#include <stdint.h>

struct usb_endpoint;

struct usb_request {
	/* XXX write me */
	uint8_t request;
};

void usb_in(struct usb_endpoint *ep, const void *buf, size_t buflen, size_t sendlen);
void usb_stall(struct usb_endpoint *ep);

#endif
