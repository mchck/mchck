#include <stdio.h>

#include <usb/usb.h>
#include <usb/dfu.h>

#include "desc.h"


static char fw_buf[4096];

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
        printf("setup_write: off %zd, len %zd\n", off, len);
        *buf = fw_buf;
        return (DFU_STATUS_OK);
};

static enum dfu_status
finish_write(void *buf, size_t off, size_t len)
{
        if (off + len > 65536)
                return (DFU_STATUS_errADDRESS);
        printf("finish_write: off %zd, len %zd\n", off, len);
        return (DFU_STATUS_OK);
}

static struct dfu_ctx dfu_ctx;

static void
init_usb_bootloader(int config)
{
        dfu_init(setup_write, finish_write, &dfu_ctx);
}

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
        USB_DESC_STRING(u"vdfu test"),
        NULL
};

const static struct usbd_config dfu_config = {
        .init = init_usb_bootloader,
        .desc = &usb_desc_config1.config, /* from desc.c */
        .function = { &dfu_function, NULL },
};

const struct usbd_device dfu_device = {
        .dev_desc = &dfu_dev_desc,
        .string_descs = dfu_string_descs,
        .configs = { &dfu_config, NULL }
};

int
main(void)
{
        usb_init(&dfu_device);
        vusb_main_loop();
}
