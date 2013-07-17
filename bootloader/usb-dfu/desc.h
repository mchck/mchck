#define USB_FUNCTION_IFACE(x) (USB_FUNCTION_IFACE_COUNT + x)

#define USB_DESC_PREP
#include <usb/dfu-desc.c>
#undef USB_DESC_PREP

extern struct usb_desc_config1 {
        struct usb_desc_config_t config;

#define USB_DESC_DECLARE
#include <usb/dfu-desc.c>
#undef USB_DESC_DECLARE
        dfu1;
} usb_desc_config1;
