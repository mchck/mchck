#include <inttypes.h>

/**
 * USB state machine
 * =================
 *
 * Setup
 * -----
 *
 * Token sequence:
 * 1.  SETUP
 * 2a. OUT
 * 3a. IN
 *
 * or
 * 1.  SETUP
 * 2b. IN
 * 3b. OUT
 *
 * Report errors by STALLing the EP (both in/out I guess) after (1) or
 * (2), so that (3) will STALL.
 *
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
 *
 */

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

enum usb_dev_class {
	USB_DEV_CLASS_SEE_IFACE = 0,
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

union usb_bcd_t {
	struct {
		uint8_t sub : 4;
		uint8_t min : 4;
		uint16_t maj : 8;
	};
	uint16_t bcd;
};
CTASSERT_SIZE_BYTE(union usb_bcd_t, 2);

struct usb_desc_dev_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_DEV */
	union usb_bcd_t usbver;	     /* = 0x0200 */
	enum usb_dev_class devclass : 8;
	enum usb_dev_subclass devsubclass : 8;
	enum usb_dev_proto : 8;
	uint8_t ep0_maxsize;
	uint16_t vid;
	uint16_t pid;
	union usb_bcd_t devver;
	uint8_t manuf_strdesc;
	uint8_t prod_strdesc;
	uint8_t serial_strdesc;
	uint8_t numconfig;
};
CTASSERT_SIZE_BYTE(struct usb_desc_dev_t, 18);

struct usb_desc_config_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_CONFIG */
	uint16_t total_length;	     /* size of config, iface, ep */
	uint8_t num_ifaces;
	uint8_t config_val;
	uint8_t config_strdesc;
	struct {
		uint8_t _rsvd0 : 5;
		uint8_t remote_wakeup : 1;
		uint8_t self_powered : 1;
		uint8_t one : 1; /* = 1 for historical reasons */
	};
	uint8_t maxpower;	/* units of 2mA */
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_config_t, 9);

enum usb_iface_class {
	USB_IFACE_CLASS_SEE_IFACE = 0,
	USB_IFACE_CLASS_VENDOR = 0xff
};

enum usb_iface_subclass {
	USB_IFACE_SUBCLASS_SEE_IFACE = 0,
	USB_IFACE_SUBCLASS_VENDOR = 0xff
};

enum usb_iface_proto {
	USB_IFACE_PROTO_SEE_IFACE = 0,
	USB_IFACE_PROTO_VENDOR = 0xff
};

struct usb_desc_iface_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_IFACE */
	uint8_t iface_num;
	uint8_t alternate;
	uint8_t num_ep;
	enum usb_iface_class : 8;
	enum usb_iface_subclass : 8;
	enum usb_iface_proto : 8;
	uint8_t iface_strdesc;
};
CTASSERT_SIZE_BYTE(struct usb_desc_iface_t, 9);

struct usb_desc_ep_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_EP */
	union {
		struct {
			uint8_t ep_num : 4;
			uint8_t _rsvd0 : 3;
			uint8_t in : 1;
		};
		uint8_t addr;
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
		uint16_t maxsize : 11;
		uint16_t _rsvd2 : 5;
	};
	uint8_t interval;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_desc_ep_t, 7);

struct usb_desc_string_t {
	uint8_t desc_length;
	enum usb_desc_type desc_type : 8; /* = USB_DESC_STRING */
	wchar_t string[];
};


/**
 * USB request data structures.
 */

struct usb_ctrl_req_t {
	union {
		struct {
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
		uint8_t reqtype;
	};
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
	} request : 8;
	uint16_t value;
	uint16_t index;
	uint16_t length;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_ctrl_req_t, 8);

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


union USB_BD_t {
	struct /* common */ {
		uint32_t _rsvd0	 : 6;
		uint32_t data1	 : 1;
		uint32_t own	 : 1;
		uint32_t _rsvd1	 : 8;
		uint32_t bc	 : 10;
		uint32_t _rsvd2	 : 6;
	};
	struct /* USB-FS */ {
		uint32_t _rsvd3	 : 2;
		uint32_t stall	 : 1;
		uint32_t dts	 : 1;
		uint32_t ninc	 : 1;
		uint32_t keep	 : 1;
		uint32_t _rsvd4	 : 26;
	};
	struct /* processor */ {
		uint32_t _rsvd5	 : 2;
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
		} tok_pid : 4;
		uint32_t _rsvd6	 : 26;
	};
	struct /* non-bitfields */ {
		uint32_t bd;
		uint32_t addr;
	};
};
CTASSERT_SIZE_BYTE(union USB_BD_t, 8);


void
usb_enable(void)
{
	/* clock distribution? */
	/* INTEN->(TOKDNE,USBRST)=1 */
	/* BDTPAGE1,2,3 */
	/* ENDPT0->(EPRXEN,EPTXEN,EPHSHK)=1 */
	/* USBCTRL->(SUSP,PDE)=0 */
	/* CTL->USBENSOFEN=1 */
}

void
usb_intr(void)
{
	/* check STAT->(ENDP,TX,ODD) */
	/* check ERRSTAT->(DMAERR) */
	/* read BDT entry */
}
