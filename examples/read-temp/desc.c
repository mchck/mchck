const static struct usb_desc_dev_t cdc_dev_desc = {
        .bLength = sizeof(struct usb_desc_dev_t),
        .bDescriptorType = USB_DESC_DEV,
        .bcdUSB = { .maj = 2 },
        .bDeviceClass = USB_DEV_CLASS_SEE_IFACE,
        .bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,
        .bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,
        .bMaxPacketSize0 = EP0_BUFSIZE,
        .idVendor = 0x2323,
        .idProduct = 3,
        .bcdDevice = { .raw = 0 },
        .iManufacturer = 1,
        .iProduct = 2,
        .iSerialNumber = 3,
        .bNumConfigurations = 1,
};

const struct usb_desc_string_t * const cdc_string_descs[] = {
        USB_DESC_STRING_LANG_ENUS,
        USB_DESC_STRING(u"mchck.org"),
        USB_DESC_STRING(u"MC HCK serial test"),
        USB_DESC_STRING_SERIALNO,
        NULL
};

const struct {
        struct usb_desc_config_t config;
        USB_FUNCTION_DESC_CDC_DECL cdc;
} cdc_desc_config = {
        .config = {
                .bLength = sizeof(struct usb_desc_config_t),
                .bDescriptorType = USB_DESC_CONFIG,
                .wTotalLength = sizeof(cdc_desc_config),
                .bNumInterfaces = (0 + USB_FUNCTION_CDC_IFACE_COUNT),
                .bConfigurationValue = 1,
                .iConfiguration = 0,
                .one = 1,
                .bMaxPower = 50
        },
        .cdc = USB_FUNCTION_DESC_CDC(0, 0, 0)
};
