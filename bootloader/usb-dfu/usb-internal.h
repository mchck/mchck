
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


struct usbd_ep_pipe_state_t {
	enum usb_ep_pingpong pingpong; /* next desc to use */
	enum usb_data01 data01;
	size_t transfer_size;
	size_t pos;
	uint8_t *data_buf;
	int short_transfer;
	ep_callback_t callback;
	void *callback_data;
	size_t ep_maxsize;
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
	enum usbd_dev_state {
		USBD_STATE_DISABLED = 0,
		USBD_STATE_DEFAULT,
		USBD_STATE_SETTING_ADDRESS,
		USBD_STATE_ADDRESS,
		USBD_STATE_CONFIGURED
	} state;
	enum usbd_ctrl_state {
		USBD_CTRL_STATE_IDLE,
		USBD_CTRL_STATE_DATA,
		USBD_CTRL_STATE_STATUS
	} ctrl_state;
	enum usb_ctrl_req_dir ctrl_dir;
	int address;
	int config;
	struct usbd_ep_state_t ep0_state;
	const struct usb_desc_dev_t *dev_desc;
	const struct usb_desc_config_t *config_desc;
	const struct usb_desc_string_t * const *string_descs;
	int (*class_control)(struct usb_ctrl_req_t *);
};
