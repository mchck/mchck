#include "vusb.h"
#include "usb.h"
#include "dfu.h"


const static struct usb_desc_dev_t dev_desc = {
        .bLength = sizeof(struct usb_desc_dev_t),
        .bDescriptorType = USB_DESC_DEV,
        .bcdUSB = { .maj = 2 },
        .bDeviceClass = USB_DEV_CLASS_SEE_IFACE,
        .bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,
        .bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,
        .bMaxPacketSize0 = EP0_BUFSIZE,
        .idVendor = 0x2323,
        .idProduct = 1,
        .bcdDevice = { .bcd = 0 },
        .iManufacturer = 1,
        .iProduct = 1,
        .iSerialNumber = 0,
        .bNumConfigurations = 1,
};

const static struct {
        struct usb_desc_config_t config;
        struct usb_desc_iface_t iface;
} __packed config_desc = {
        .config = {
                .bLength = sizeof(struct usb_desc_config_t),
                .bDescriptorType = USB_DESC_CONFIG,
                .wTotalLength = sizeof(config_desc),
                .bNumInterfaces = 1,
                .bConfigurationValue = 0,
                .iConfiguration = 0,
                .remote_wakeup = 0,
                .self_powered = 0,
                .one = 1,
                .bMaxPower = 50
        },
        .iface = {
                .bLength = sizeof(struct usb_desc_iface_t),
                .bDescriptorType = USB_DESC_IFACE,
                .bInterfaceNumber = 0,
                .bAlternateSetting = 0,
                .bNumEndpoints = 0,
                .bInterfaceClass = USB_DEV_CLASS_VENDOR,
                .bInterfaceSubClass = 0,
                .bInterfaceProtocol = 0,
                .iInterface = 0,
        }
};

const static struct usb_desc_string_t * const string_descs[] = {
        USB_DESC_STRING_LANG_ENUS,
        USB_DESC_STRING(u"MCHCK test"),
        NULL
};


static char fw_buf[4096];

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
        if (len == 0 && off != 65536)
                return (DFU_STATUS_errNOTDONE);
        printf("setup_write: off %zd, len %zd\n", off, len);
        *buf = fw_buf;
        return (DFU_STATUS_OK);
};

static enum dfu_status
finish_write(size_t off, size_t len)
{
        if (off + len > 65536)
                return (DFU_STATUS_errADDRESS);
        printf("finish_write: off %zd, len %zd\n", off, len);
        dfu_write_done(DFU_STATUS_OK);
        return (DFU_STATUS_OK);
}

int
main(void)
{
        dfu_start(setup_write, finish_write);
        vusb_main_loop();
}
