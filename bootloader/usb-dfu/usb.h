#include <sys/types.h>

#include <inttypes.h>
#include <string.h>
#include <uchar.h>

/**
 * Note: bitfields ahead.
 * GCC fills the fields lsb-to-msb on little endian.
 */

#define __packed __attribute__((__packed__))

/* From FreeBSD: compile-time asserts */
#define CTASSERT(x)             _CTASSERT(x, __LINE__)
#define _CTASSERT(x, y)         __CTASSERT(x, y)
#define __CTASSERT(x, y)        typedef char __assert ## y[(x) ? 1 : -1]

#define CTASSERT_SIZE_BYTE(t, s)     CTASSERT(sizeof(t) == (s))
#define CTASSERT_SIZE_BIT(t, s)     CTASSERT(sizeof(t) * 8 == (s))


/**
 * USB descriptors
 */

enum usb_desc_type {
	USB_DESC_DEV = 1,
	USB_DESC_CONFIG = 2,
	USB_DESC_STRING = 3,
	USB_DESC_IFACE = 4,
	USB_DESC_EP = 5,
	USB_DESC_DEVQUAL = 6,
	USB_DESC_OTHERSPEED = 7,
	USB_DESC_POWER = 8
};

union usb_desc_type_t {
	struct {
		uint8_t id : 5;
		enum usb_desc_type_type {
			USB_DESC_TYPE_STD = 0,
			USB_DESC_TYPE_CLASS = 1,
			USB_DESC_TYPE_VENDOR = 2
		} type_type : 2;
		uint8_t _rsvd0 : 1;
	} __packed;
	uint8_t type;
} __packed;
CTASSERT_SIZE_BYTE(union usb_desc_type_t, 1);

enum usb_dev_class {
	USB_DEV_CLASS_SEE_IFACE = 0,
	USB_DEV_CLASS_APP = 0xfe,
	USB_DEV_CLASS_VENDOR = 0xff
};

enum usb_dev_subclass {
	USB_DEV_SUBCLASS_SEE_IFACE = 0,
	USB_DEV_SUBCLASS_APP_DFU = 0x01,
	USB_DEV_SUBCLASS_VENDOR = 0xff
};

enum usb_dev_proto {
	USB_DEV_PROTO_SEE_IFACE = 0,
	USB_DEV_PROTO_DFU = 0x01,
	USB_DEV_PROTO_VENDOR = 0xff
};

union usb_bcd_t {
	struct {
		uint8_t sub : 4;
		uint8_t min : 4;
		uint16_t maj : 8;
	};
	uint16_t bcd;
};
CTASSERT_SIZE_BYTE(union usb_bcd_t, 2);

struct usb_desc_generic_t {
	uint8_t bLength;
	union usb_desc_type_t bDescriptorType;
	uint8_t data[];
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_generic_t, 2);

struct usb_desc_dev_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_DEV */
	union usb_bcd_t bcdUSB;	     /* = 0x0200 */
	enum usb_dev_class bDeviceClass : 8;
	enum usb_dev_subclass bDeviceSubClass : 8;
	enum usb_dev_proto bDeviceProtocol : 8;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	union usb_bcd_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_dev_t, 18);

struct usb_desc_ep_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_EP */
	union {
		struct {
			uint8_t ep_num : 4;
			uint8_t _rsvd0 : 3;
			uint8_t in : 1;
		};
		uint8_t bEndpointAddress;
	};
	struct {
		enum usb_ep_type {
			USB_EP_CONTROL = 0,
			USB_EP_ISO = 1,
			USB_EP_BULK = 2,
			USB_EP_INTR = 3
		} type : 2;
		enum usb_ep_iso_synctype {
			USB_EP_ISO_NOSYNC = 0,
			USB_EP_ISO_ASYNC = 1,
			USB_EP_ISO_ADAPTIVE = 2,
			USB_EP_ISO_SYNC = 3
		} sync_type : 2;
		enum usb_ep_iso_usagetype {
			USB_EP_ISO_DATA = 0,
			USB_EP_ISO_FEEDBACK = 1,
			USB_EP_ISO_IMPLICIT = 2
		} usage_type : 2;
		uint8_t _rsvd1 : 2;
	} __packed;
	struct {
		uint16_t wMaxPacketSize : 11;
		uint16_t _rsvd2 : 5;
	};
	uint8_t bInterval;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_ep_t, 7);

struct usb_desc_iface_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_IFACE */
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	enum usb_dev_class bInterfaceClass : 8;
	enum usb_dev_subclass bInterfaceSubClass: 8;
	enum usb_dev_proto bInterfaceProtocol : 8;
	uint8_t iInterface;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_iface_t, 9);

struct usb_desc_config_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_CONFIG */
	uint16_t wTotalLength;	     /* size of config, iface, ep */
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	struct {
		uint8_t _rsvd0 : 5;
		uint8_t remote_wakeup : 1;
		uint8_t self_powered : 1;
		uint8_t one : 1; /* = 1 for historical reasons */
	};
	uint8_t bMaxPower;	/* units of 2mA */
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_config_t, 9);

struct usb_desc_string_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_STRING */
	const char16_t bString[];
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_string_t, 2);

#define USB_DESC_STRING(s)					\
        (void *)&(struct {					\
			struct usb_desc_string_t dsc;		\
			char16_t str[sizeof(s) / 2 - 1];	\
        } __packed) {{						\
			.bLength = sizeof(struct usb_desc_string_t) +	\
				sizeof(s) - 2,			\
			.bDescriptorType = USB_DESC_STRING,	\
			},					\
			s					\
	}
#define USB_DESC_STRING_LANG_ENUS USB_DESC_STRING(u"\x0409")

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

struct usb_ctrl_req_t {
	union /* reqtype and request & u16 */ {
		struct /* reqtype and request */ {
			union /* reqtype in bitfield & u8 */ {
				struct /* reqtype */ {
					enum usb_ctrl_req_recp {
						USB_CTRL_REQ_DEV = 0,
						USB_CTRL_REQ_IFACE = 1,
						USB_CTRL_REQ_EP = 2,
						USB_CTRL_REQ_OTHER = 3
					} recp : 5;
					enum usb_ctrl_req_type {
						USB_CTRL_REQ_STD = 0,
						USB_CTRL_REQ_CLASS = 1,
						USB_CTRL_REQ_VENDOR = 2
					} type : 2;
					enum usb_ctrl_req_dir {
						USB_CTRL_REQ_OUT = 0,
						USB_CTRL_REQ_IN = 1
					} in : 1;
				} __packed;
				uint8_t bmRequestType;
			}; /* union */
			enum usb_ctrl_req_code {
				USB_CTRL_REQ_GET_STATUS = 0,
				USB_CTRL_REQ_CLEAR_FEATURE = 1,
				USB_CTRL_REQ_SET_FEATURE = 3,
				USB_CTRL_REQ_SET_ADDRESS = 5,
				USB_CTRL_REQ_GET_DESCRIPTOR = 6,
				USB_CTRL_REQ_SET_DESCRIPTOR = 7,
				USB_CTRL_REQ_GET_CONFIGURATION = 8,
				USB_CTRL_REQ_SET_CONFIGURATION = 9,
				USB_CTRL_REQ_GET_INTERFACE = 10,
				USB_CTRL_REQ_SET_INTERFACE = 11,
				USB_CTRL_REQ_SYNC_FRAME = 12
			} bRequest : 8;
		} __packed; /* struct */
		uint16_t type_and_req;
	}; /* union */
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_ctrl_req_t, 8);

#define USB_CTRL_REQ_DIR_SHIFT 0
#define USB_CTRL_REQ_TYPE_SHIFT 1
#define USB_CTRL_REQ_RECP_SHIFT 3
#define USB_CTRL_REQ_CODE_SHIFT 8
#define USB_CTRL_REQ(req_inout, req_type, req_code)			\
	(uint16_t)							\
	((USB_CTRL_REQ_##req_inout << USB_CTRL_REQ_DIR_SHIFT)		\
	 |(USB_CTRL_REQ_##req_type << USB_CTRL_REQ_TYPE_SHIFT)		\
	 |(USB_CTRL_REQ_##req_code << USB_CTRL_REQ_CODE_SHIFT))

/**
 * status replies for GET_STATUS.
 */

struct usb_ctrl_req_status_dev_t {
	uint16_t self_powered : 1;
	uint16_t remote_wakeup : 1;
	uint16_t _rsvd : 14;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_dev_t, 16);

struct usb_ctrl_req_status_iface_t {
	uint16_t _rsvd;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_iface_t, 16);

struct usb_ctrl_req_status_ep_t {
	uint16_t halt : 1;
	uint16_t _rsvd : 15;
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_status_ep_t, 16);

/**
 * Descriptor type (in req->value) for GET_DESCRIPTOR.
 */
struct usb_ctrl_req_desc_t {
	uint8_t idx;
	enum usb_desc_type type : 8;
} __packed;
CTASSERT_SIZE_BIT(struct usb_ctrl_req_desc_t, 16);

/**
 * Feature selector (in req->value) for CLEAR_FEATURE.
 */
enum usb_ctrl_req_feature {
	USB_CTRL_REQ_FEAT_EP_HALT = 0,
	USB_CTRL_REQ_FEAT_DEV_REMOTE_WKUP = 1,
	USB_CTRL_REQ_FEAT_TEST_MODE = 2
};


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


/**
 * Kinetis USB driver notes:
 * We need to manually maintain the DATA0/1 toggling for the SIE.
 * SETUP transactions always start with a DATA0.
 *
 * The SIE internally uses pingpong (double) buffering, which is
 * easily confused with DATA0/DATA1 toggling, and I think even the
 * Freescale docs confuse the two notions.  When BD->DTS is set,
 * BD->DATA01 will be used to verify/discard incoming DATAx and it
 * will set the DATAx PID for outgoing tokens.  This is not described
 * as such in the Freescale Kinetis docs, but the Microchip PIC32 OTG
 * docs are more clear on this;  it seems that both Freescale and
 * Microchip use different versions of the same USB OTG IP core.
 *
 * http://ww1.microchip.com/downloads/en/DeviceDoc/61126F.pdf
 *
 * Clear CTL->TOKEN_BUSY after SETUP tokens.
 */

#define EP0_BUFSIZE 64

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

enum usbd_dev_state {
	USBD_STATE_DISABLED = 0,
	USBD_STATE_DEFAULT,
	USBD_STATE_SETTING_ADDRESS,
	USBD_STATE_ADDRESS,
	USBD_STATE_CONFIGURED
};

typedef void (*ep_callback_t)(void *buf, ssize_t len, void *data);


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
	struct USB_BD_t *bdt;
	enum usbd_dev_state state;
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
	uint8_t ep0_buf[EP0_BUFSIZE][2];
};

struct usb_xfer_info;


void usb_enable(void);
void *usb_get_xfer_data(struct usb_xfer_info *);
enum usb_tok_pid usb_get_xfer_pid(struct usb_xfer_info *);
void usb_enable_xfers(void);
void usb_set_addr(int);
void usb_ep_stall(int);
void usb_clear_transfers(void);
size_t usb_ep_get_transfer_size(int, enum usb_ep_dir, enum usb_ep_pingpong);
void usb_tx_queue_next(struct usbd_ep_pipe_state_t *, void *, size_t);
void usb_rx_queue_next(struct usbd_ep_pipe_state_t *, void *, size_t);

void usb_start(const struct usb_desc_dev_t *, const struct usb_desc_config_t *, const struct usb_desc_string_t * const *);
void usb_handle_transaction(struct usb_xfer_info *);
