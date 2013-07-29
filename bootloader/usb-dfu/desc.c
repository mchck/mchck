const static struct usb_desc_dev_t dfu_dev_desc = {
        .bLength = sizeof(struct usb_desc_dev_t),
        .bDescriptorType = USB_DESC_DEV,
        .bcdUSB = { .maj = 2 },
        .bDeviceClass = USB_DEV_CLASS_SEE_IFACE,
        .bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,
        .bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,
        .bMaxPacketSize0 = EP0_BUFSIZE,
        .idVendor = 0x2323,
        .idProduct = 1,
        .bcdDevice = { .raw = 0 },
        .iManufacturer = 1,
        .iProduct = 2,
        .iSerialNumber = 0,
        .bNumConfigurations = 1,
};

const struct usb_desc_string_t * const dfu_string_descs[] = {
        USB_DESC_STRING_LANG_ENUS,
        USB_DESC_STRING(u"mchck.org"),
        USB_DESC_STRING(u"MC HCK bootloader"),
        NULL
};

const struct {
        struct usb_desc_config_t config;
        USB_FUNCTION_DESC_DFU_DECL dfu;
} dfu_desc_config = {
        .config = {
                .bLength = sizeof(struct usb_desc_config_t),
                .bDescriptorType = USB_DESC_CONFIG,
                .wTotalLength = sizeof(dfu_desc_config),
                .bNumInterfaces = (0 + USB_FUNCTION_DFU_IFACE_COUNT),
                .bConfigurationValue = 1,
                .iConfiguration = 0,
                .one = 1,
                .bMaxPower = 50
        },
        .dfu = USB_FUNCTION_DESC_DFU(0, 0, 0)
};

const static struct usbd_config dfu_config = {
        .init = init_usb_bootloader,
        .desc = &dfu_desc_config.config,
        .function = { &dfu_function, NULL },
};

const struct usbd_device dfu_device = {
        .dev_desc = &dfu_dev_desc,
        .string_descs = dfu_string_descs,
        .configs = { &dfu_config, NULL }
};
