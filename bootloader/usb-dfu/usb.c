#include <inttypes.h>
#include <wchar.h>

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
} __packed;
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
} __packed;
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
	} __packed;
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
				uint8_t reqtype;
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
			} request : 8;
		} __packed; /* struct */
		uint16_t type_and_req;
	}; /* union */
	uint16_t value;
	uint16_t index;
	uint16_t length;
} __packed;
CTASSERT_SIZE_BYTE(struct usb_ctrl_req_t, 8);

#define USB_CTRL_REQ_DIR_SHIFT 0
#define USB_CTRL_REQ_TYPE_SHIFT 1
#define USB_CTRL_REQ_RECP_SHIFT 3
#define USB_CTRL_REQ_CODE_SHIFT 8
#define USB_CTRL_REQ(req_inout, req_type, req_code)		\
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
 * Hardware structures
 */

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

enum usbd_dev_state {
	USBD_STATE_DISABLED = 0,
	USBD_STATE_DEFAULT,
	USBD_STATE_ADDRESS,
	USBD_STATE_CONFIGURED
};

struct usbd_t {
	union USB_BD_t *bdt;
	enum usbd_dev_state state;
	enum usbd_ctrl_state {
		USBD_CTRL_STATE_IDLE,
		USBD_CTRL_STATE_WAIT_IN,
		USBD_CTRL_STATE_WAIT_OUT,
		USBD_CTRL_STATE_ERROR
	} ctrl_state;
};


struct usbd_t usbd;


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

/**
 *
 * Great resource: http://wiki.osdev.org/Universal_Serial_Bus
 *
 * Setup
 * -----
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

int
usb_handle_control(struct usb_ctrl_req_t *req)
{
	if (req->type != USB_CTRL_REQ_STD) {
		/* XXX pass on to higher levels */
		goto err;
	}

	/* Only STD requests here */
	switch (req->request) {
	case USB_CTRL_REQ_GET_STATUS:
		break;
	case USB_CTRL_REQ_CLEAR_FEATURE:
		break;
	case USB_CTRL_REQ_SET_FEATURE:
		break;
	case USB_CTRL_REQ_SET_ADDRESS:
		usb_set_address(req->value);
		/* XXX reset configuration */
		break;
	case USB_CTRL_REQ_GET_DESCRIPTOR:
		/* return descriptor */
		break;
	case USB_CTRL_REQ_GET_CONFIGURATION:
		break;
	case USB_CTRL_REQ_SET_CONFIGURATION:
		break;
	case USB_CTRL_REQ_GET_INTERFACE:
		break;
	case USB_CTRL_REQ_SET_INTERFACE:
		break;
	default:
		goto err;
	}

	return (0);

err:
	usb_stall_ep(0);
	return (-1);
}
