#include <usb/usb.h>
#include <usb/dfu.h>

struct dfu_desc_functional {
        uint8_t bLength;
        struct usb_desc_type_t bDescriptorType; /* = class DFU/0x1 FUNCTIONAL */
        union {
                struct {
                        uint8_t can_download : 1;
                        uint8_t can_upload : 1;
                        uint8_t manifestation_tolerant : 1;
                        uint8_t will_detach : 1;
                        uint8_t _rsvd0 : 4;
                };
                uint8_t bmAttributes;
        };
        uint16_t wDetachTimeOut;
        uint16_t wTransferSize;
        struct usb_bcd_t bcdDFUVersion;
} __packed;
CTASSERT_SIZE_BYTE(struct dfu_desc_functional, 9);

struct dfu_function_desc {
        struct usb_desc_iface_t iface;
        struct dfu_desc_functional dfu;
};
