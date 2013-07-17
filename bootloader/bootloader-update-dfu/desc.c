#include "desc.h"

struct usb_desc_config1 usb_desc_config1 = {
        .config = {
                .bLength = sizeof(struct usb_desc_config_t),
                .bDescriptorType = USB_DESC_CONFIG,
                .wTotalLength = sizeof(usb_desc_config1),
                .bNumInterfaces = (0 + USB_FUNCTION_DFU_IFACE_COUNT),
                .bConfigurationValue = 1,
                .iConfiguration = 0,
                .one = 1,
                .bMaxPower = 50
        },

        .dfu1 =
#define USB_DESC_DEFINE
#define USB_FUNCTION_IFACE_COUNT 0
#include <usb/dfu-desc.c>
#undef USB_FUNCTION_IFACE_COUNT
#define USB_FUNCTION_IFACE_COUNT (0 + USB_FUNCTION_DFU_IFACE_COUNT)
#undef USB_DESC_DEFINE
        ,
};
