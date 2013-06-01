#ifndef __USB_H
#define __USB_H

#include <sys/types.h>

#include <mchck.h>
#include <usb/usb-common.h>

/**
 * Note: bitfields ahead.
 * GCC fills the fields lsb-to-msb on little endian.
 */


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

struct usb_desc_type_t {
	UNION_STRUCT_START(8);
	uint8_t id : 5;
	enum usb_desc_type_type {
		USB_DESC_TYPE_STD = 0,
		USB_DESC_TYPE_CLASS = 1,
		USB_DESC_TYPE_VENDOR = 2
	} type_type : 2;
	uint8_t _rsvd0 : 1;
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BYTE(struct usb_desc_type_t, 1);

enum usb_dev_class {
	USB_DEV_CLASS_SEE_IFACE = 0,
	USB_DEV_CLASS_APP = 0xfe,
	USB_DEV_CLASS_VENDOR = 0xff
};

enum usb_dev_subclass {
	USB_DEV_SUBCLASS_SEE_IFACE = 0,
	USB_DEV_SUBCLASS_VENDOR = 0xff
};

enum usb_dev_proto {
	USB_DEV_PROTO_SEE_IFACE = 0,
	USB_DEV_PROTO_VENDOR = 0xff
};

struct usb_bcd_t {
	UNION_STRUCT_START(16);
	struct {
		uint8_t sub : 4;
		uint8_t min : 4;
		uint16_t maj : 8;
	};
	UNION_STRUCT_END;
};
CTASSERT_SIZE_BYTE(struct usb_bcd_t, 2);

struct usb_desc_generic_t {
	uint8_t bLength;
	struct usb_desc_type_t bDescriptorType;
	uint8_t data[];
};
CTASSERT_SIZE_BYTE(struct usb_desc_generic_t, 2);

struct usb_desc_dev_t {
	uint8_t bLength;
	enum usb_desc_type bDescriptorType : 8; /* = USB_DESC_DEV */
	struct usb_bcd_t bcdUSB;	     /* = 0x0200 */
	enum usb_dev_class bDeviceClass : 8;
	enum usb_dev_subclass bDeviceSubClass : 8;
	enum usb_dev_proto bDeviceProtocol : 8;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	struct usb_bcd_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
};
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
	};
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
};
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
};
CTASSERT_SIZE_BYTE(struct usb_desc_string_t, 2);

#define USB_DESC_STRING(s)					\
        (const void *)&(const struct {				\
			struct usb_desc_string_t dsc;		\
			char16_t str[sizeof(s) / 2 - 1];	\
        }) {{						\
			.bLength = sizeof(struct usb_desc_string_t) +	\
				sizeof(s) - 2,			\
			.bDescriptorType = USB_DESC_STRING,	\
			},					\
			s					\
	}
#define USB_DESC_STRING_LANG_ENUS USB_DESC_STRING(u"\x0409")


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
				};
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
		}; /* struct */
		uint16_t type_and_req;
	}; /* union */
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};
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
};
CTASSERT_SIZE_BIT(struct usb_ctrl_req_desc_t, 16);

/**
 * Feature selector (in req->value) for CLEAR_FEATURE.
 */
enum usb_ctrl_req_feature {
	USB_CTRL_REQ_FEAT_EP_HALT = 0,
	USB_CTRL_REQ_FEAT_DEV_REMOTE_WKUP = 1,
	USB_CTRL_REQ_FEAT_TEST_MODE = 2
};


struct usbd_identity_t {
	const struct usb_desc_dev_t *dev_desc;
	const struct usb_desc_config_t *config_desc;
	const struct usb_desc_string_t * const *string_descs;
	void (*class_control)(struct usb_ctrl_req_t *);
};

struct usb_xfer_info;
enum usb_ep_dir;
enum usb_ep_pingpong;
struct usbd_ep_pipe_state_t;
typedef void (*ep_callback_t)(void *buf, ssize_t len, void *data);


/* Provided by MD code */
void *usb_get_xfer_data(struct usb_xfer_info *);
enum usb_tok_pid usb_get_xfer_pid(struct usb_xfer_info *);
int usb_get_xfer_ep(struct usb_xfer_info *);
enum usb_ep_dir usb_get_xfer_dir(struct usb_xfer_info *);
void usb_enable_xfers(void);
void usb_set_addr(int);
void usb_ep_stall(int);
void usb_clear_transfers(void);
size_t usb_ep_get_transfer_size(struct usbd_ep_pipe_state_t *);
void usb_queue_next(struct usbd_ep_pipe_state_t *, void *, size_t);
void usb_pipe_stall(struct usbd_ep_pipe_state_t *);
void usb_pipe_unstall(struct usbd_ep_pipe_state_t *);
#ifdef VUSB
void vusb_main_loop(void);
#else
void usb_intr(void);
#endif

/* Provided by MI code */
void usb_start(const struct usbd_identity_t *);
void usb_handle_transaction(struct usb_xfer_info *);
void usb_setup_control(void);
void usb_handle_control_status(int);
int usb_rx(void *, size_t, ep_callback_t, void *);
int usb_tx(const void *, size_t, size_t, ep_callback_t, void *);
int usb_tx_cp(const void *, size_t, size_t, ep_callback_t, void *);

#endif
