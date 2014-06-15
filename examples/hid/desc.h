// Required fwd declaration:
//   void init_my_hid(inf config)

struct usb_config_hid {
	struct usb_desc_config_t config;
	struct hid_function_desc usb_function_0;
} __packed;

#define MOUSE_TX_SIZE 8

// Mouse example pg. 71
#define REPORT_DESC_SIZE 50
static uint8_t report_desc[REPORT_DESC_SIZE] = {
	0x05, 0x01,
	0x09, 0x02,
	0xA1, 0x01,
	0x09, 0x01,
	0xA1, 0x00,
	0x05, 0x09,
	0x19, 0x01,
	0x29, 0x03,
	0x15, 0x00,
	0x25, 0x01,
	0x95, 0x03,
	0x75, 0x01,
	0x81, 0x02,
	0x95, 0x01,
	0x75, 0x05,
	0x81, 0x01,
	0x05, 0x01,
	0x09, 0x30,
	0x09, 0x31,
	0x15, 0x81,
	0x25, 0x7F,
	0x75, 0x08,
	0x95, 0x02,
	0x81, 0x06,
	0xC0,
	0xC0
};

struct mouse_data_t {
	uint8_t btn1 : 1;
	uint8_t btn2 : 1;
	uint8_t btn3 : 1;
	uint8_t pad : 5;
	char x;
	char y;
} __packed;

static struct mouse_data_t mouse_data = {
	.btn1 = 0,
	.btn2 = 0,
	.btn3 = 0,
	.pad = 0,
	.x = 20,
	.y = 0
};

static const struct usb_config_hid hid_desc_config = {
	.config = {
		.bLength = sizeof(struct usb_desc_config_t),
		.bDescriptorType = USB_DESC_CONFIG,
		.wTotalLength = sizeof(struct usb_config_hid),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.one = 1,
		.bMaxPower = 50
	},
	.usb_function_0 = {
		.iface = {
			.bLength = sizeof(struct usb_desc_iface_t),
			.bDescriptorType = 0x4,
			.bInterfaceNumber = 0x0,
			.bAlternateSetting = 0x0,
			.bNumEndpoints = 0x1,
			.bInterfaceClass = 0x3,
			.bInterfaceSubClass = 0x0,
			.bInterfaceProtocol = 0x2,
			.iInterface = 0x0
		},
		.hid_desc = {
			.bLength = 0x9,
			.bDescriptorType = 0x21,
			.bcdHID = 0x101,
			.bCountryCode = 0x0,
			.bNumDescriptors = 0x1,
			.bDescriptorType1 = 0x22,
			.wDescriptorLength = 0x32,
		},
		.int_in_ep = {
			.bLength = 0x7,
			.bDescriptorType = 0x5,
			.ep_num = 1,
			.in = 1,
			.type = USB_EP_INTR,
			.wMaxPacketSize = MOUSE_TX_SIZE,
			.bInterval = 0xA
		}
	}
};

static const struct usbd_function hid_usbd_function = {
};

static const struct usbd_config usbd_hid_config = {
	.init = init_my_hid,
	.desc = &hid_desc_config.config,
	.function = { &hid_usbd_function }
};

static const struct usb_desc_string_t * const hid_usb_strings[] = {
	USB_DESC_STRING_LANG_ENUS,
	USB_DESC_STRING(u"mchck"),
	USB_DESC_STRING(u"hid test"),
	USB_DESC_STRING_SERIALNO,
	NULL
};

static const struct usb_desc_dev_t
hid_usb_dev_desc = {
	.bLength = sizeof(struct usb_desc_dev_t),
	.bDescriptorType = USB_DESC_DEV,
	.bcdUSB = { .maj = 2 },
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = EP0_BUFSIZE,
	.idVendor = 0x2323,
	.idProduct = 3,
	.bcdDevice = { .raw = 0 },
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

static const struct usbd_device hid_dev = {
	.dev_desc = &hid_usb_dev_desc,
	.string_descs = hid_usb_strings,
	.configs = {
		&usbd_hid_config,
		NULL
	}
};
