#ifndef _USB_HID_H
#define _USB_HID_H

#include <sys/types.h>


#define HID_TX_EP	1
#define HID_RX_EP	2

enum hid_report_descriptor_type {
	USB_HID_REPORT_DESC_TYPE_HID = 0x21,
	USB_HID_REPORT_DESC_TYPE_REPORT = 0x22,
	USB_HID_REPORT_DESC_TYPE_PHYSICAL = 0x23
};

enum hid_report_type {
	USB_HID_REPORT_TYPE_INPUT = 0x01,
	USB_HID_REPORT_TYPE_OUTPUT = 0x02,
	USB_HID_REPORT_TYPE_FEATURE = 0x03
};

enum hid_protocol_t {
	USB_HID_PROTOCOL_BOOT = 0x0,
	USB_HID_PROTOCOL_REPORT = 0x1
};

/* standard requests */
typedef void (*hid_set_report_descriptor_t)(const enum hid_report_descriptor_type type, const uint8_t index, const void *data_in, const size_t data_len);
typedef size_t (*hid_get_report_descriptor_t)(enum hid_report_descriptor_type type, uint8_t index, void **data_out);
/* class specific requests */
typedef void (*hid_set_report_t)(const enum hid_report_type type, const uint8_t report_id, const void *data_in, const size_t data_len);
typedef size_t (*hid_get_report_t)(enum hid_report_type type, uint8_t report_id, void **data_out);
typedef void (*hid_set_idle_t)(const uint8_t duration, const uint8_t report_id);
typedef uint8_t (*hid_get_idle_t)(uint8_t report_id);
typedef void (*hid_set_protocol_t)(const enum hid_protocol_t protocol);
typedef enum hid_protocol_t (*hid_get_protocol_t)();

// XXX probably rename this
struct hid_user_functions_t {
	hid_get_report_descriptor_t get_descriptor; // REQUIRED
	hid_get_report_t get_report; // REQUIRED
	hid_set_report_t set_report;
	hid_get_idle_t get_idle;
	hid_set_idle_t set_idle;
	hid_get_protocol_t get_protocol;
	hid_set_protocol_t set_protocol;
};

struct hid_ctx {
	struct usbd_function_ctx_header header;
	struct hid_user_functions_t *user_functions;
	struct usbd_ep_pipe_state_t *tx_pipe;
	struct usbd_ep_pipe_state_t *rx_pipe;
};

void hid_init(struct hid_user_functions_t *, struct hid_ctx *);
void hid_send_data(struct hid_ctx *, void *, size_t, size_t, ep_callback_t, void *);
void hid_recv_data(struct hid_ctx *, void **, size_t, ep_callback_t, void *);

#endif
