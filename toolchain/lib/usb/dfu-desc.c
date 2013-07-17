#if defined(USB_DESC_PREP)
#include "dfu-desc.h"

#elif defined(USB_DESC_DECLARE)
struct dfu_function_desc

#elif defined(USB_DESC_DEFINE)
{
        .iface = {
                .bLength = sizeof(struct usb_desc_iface_t),
                .bDescriptorType = USB_DESC_IFACE,
                .bInterfaceNumber = USB_FUNCTION_IFACE(0),
                .bAlternateSetting = 0,
                .bNumEndpoints = 0,
                .bInterfaceClass = USB_DEV_CLASS_APP,
                .bInterfaceSubClass = USB_DEV_SUBCLASS_APP_DFU,
                .bInterfaceProtocol = USB_DEV_PROTO_DFU_DFU,
                .iInterface = 0,
        },
        .dfu = {
                .bLength = sizeof(struct dfu_desc_functional),
                .bDescriptorType = {
                        .id = 0x1,
                        .type_type = USB_DESC_TYPE_CLASS
                },
                .will_detach = 1,
                .manifestation_tolerant = 0,
                .can_upload = 0,
                .can_download = 1,
                .wDetachTimeOut = 0,
                .wTransferSize = USB_DFU_TRANSFER_SIZE,
                .bcdDFUVersion = { .maj = 1, .min = 1 }
        }
}
#else
#error "Unknown descriptor generation mode"
#endif
