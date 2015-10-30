
/**
 * Internal driver structures
 */

/**
 * USB state machine
 * =================
 *
 * Device configuration states:
 *
 * Attached <-> Powered
 * Powered -(reset)-> Default
 * Default -(SET_ADDRESS)-> Address
 * Address -(SET_CONFIGURATION)-> Configured
 * Configured -(SET_CONFIGURATION 0)-> Address
 * Address -(SET_ADDRESS 0)-> Default
 * [Default, Configured, Address] -(reset)-> Default
 */


#ifndef USB_MAX_EP
#define USB_MAX_EP 16
#endif

struct usbd_ep_pipe_state_t {
	enum usb_ep_pingpong pingpong; /* next desc to use */
	enum usb_data01 data01;
	size_t transfer_size;
	size_t pos;
	uint8_t *data_buf;
	const uint8_t *copy_source;
	int short_transfer;
	ep_callback_t callback;
	void *callback_data;
	size_t ep_maxsize;
	/* constant */
	int ep_num;
	enum usb_ep_dir ep_dir;
};

struct usbd_ep_state_t {
	union {
		struct usbd_ep_pipe_state_t pipe[2];
		struct {
			struct usbd_ep_pipe_state_t rx;
			struct usbd_ep_pipe_state_t tx;
		};
	};
};

struct usbd_t {
	struct usbd_function_ctx_header functions;
	struct usbd_function control_function;
	const struct usbd_device *identity;
	int address;
	int config;
	enum usbd_dev_state {
		USBD_STATE_DISABLED = 0,
		USBD_STATE_DEFAULT,
		USBD_STATE_SETTING_ADDRESS,
		USBD_STATE_ADDRESS,
		USBD_STATE_CONFIGURED
	} state;
	enum usb_ctrl_req_dir ctrl_dir;
	struct usbd_ep_state_t ep_state[USB_MAX_EP];
};

extern struct usbd_t usb;

void usb_restart(void);
void usb_enable(void);
const struct usbd_config *usb_get_config_data(int config);
