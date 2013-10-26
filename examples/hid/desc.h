// Required fwd declaration:
//   void init_my_hid(inf config)

struct hid_desc_t {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bDescriptorType1;
	uint16_t wDescriptorLength;
} __packed;

struct hid_function_desc {
	struct usb_desc_iface_t iface;
	struct hid_desc_t hid_desc;
	struct usb_desc_ep_t int_in_ep;
} __packed;

struct usb_config_hid {
	struct usb_desc_config_t config;
	struct hid_function_desc usb_function_0;
} __packed;

static const struct usb_config_hid hid_desc_config = {
	.config = {
		.bLength = sizeof(struct usb_desc_config_t),
		.bDescriptorType = USB_DESC_CONFIG,
		.wTotalLength = sizeof(struct usb_config_hid),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.one = 1,
		.bMaxPower = 100
	},
	.usb_function_0 = {
		.iface = {
			.bLength = sizeof(struct usb_desc_iface_t),
			.bDescriptorType = USB_DESC_IFACE,
			.bInterfaceNumber = 0,
			.bAlternateSetting = 0,
			.bNumEndpoints = 1,
			.bInterfaceClass = 3,
			.bInterfaceSubClass = 0,
			.bInterfaceProtocol = 0,
			.iInterface = 0
		},
		.hid_desc = {
			.bLength = 9,
			.bDescriptorType = 0x21,
			.bcdHID = 0x101,
			.bCountryCode = 0,
			.bNumDescriptors = 1,
			.bDescriptorType1 = 0x22,
			.wDescriptorLength = 0x32,
		},
		.int_in_ep = {
			.bLength = sizeof(struct usb_desc_ep_t),
			.bDescriptorType = USB_DESC_EP,
			.ep_num = 1,
			.in = 1,
			.type = USB_EP_INTR,
			.wMaxPacketSize = 8,
			.bInterval = 0x0A
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
	.bDeviceClass = USB_DEV_CLASS_SEE_IFACE,
	.bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,
	.bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,
	.bMaxPacketSize0 = EP0_BUFSIZE,
	.idVendor = 8995,
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
