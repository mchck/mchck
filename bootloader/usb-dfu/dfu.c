#include "usb.h"

enum dfu_ctrl_req_code {
        USB_CTRL_REQ_DFU_DETACH = 0,
        USB_CTRL_REQ_DFU_DNLOAD = 1,
        USB_CTRL_REQ_DFU_UPLOAD = 2,
        USB_CTRL_REQ_DFU_GETSTATUS = 3,
        USB_CTRL_REQ_DFU_CLRSTATUS = 4,
        USB_CTRL_REQ_DFU_GETSTATE = 5,
        USB_CTRL_REQ_DFU_ABORT = 6
};

struct dfu_desc_function_t {
        uint8_t bLength;
        union usb_desc_type_t bDescriptorType; /* = class DFU/0x1 FUNCTIONAL */
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
        union usb_bcd_t bcdDFUVersion;
} __packed;
CTASSERT_SIZE_BYTE(struct dfu_desc_function_t, 9);

enum dfu_status {
        DFU_STATUS_OK = 0x00,
        DFU_STATUS_errTARGET,
        DFU_STATUS_errFILE,
        DFU_STATUS_errWRITE,
        DFU_STATUS_errERASE,
        DFU_STATUS_errCHECK_ERASED,
        DFU_STATUS_errPROG,
        DFU_STATUS_errVERIFY,
        DFU_STATUS_errADDRESS,
        DFU_STATUS_errNOTDONE,
        DFU_STATUS_errFIRMWARE,
        DFU_STATUS_errVENDOR,
        DFU_STATUS_errUSBR,
        DFU_STATUS_errPOR,
        DFU_STATUS_errUNKNOWN,
        DFU_STATUS_errSTALLEDPKT
};

enum dfu_state {
        DFU_STATE_appIDLE = 0,
        DFU_STATE_appDETACH,
        DFU_STATE_dfuDNLOAD_SYNC,
        DFU_STATE_dfuDNBUSY,
        DFU_STATE_dfuDNLOAD_IDLE,
        DFU_STATE_dfuMANIFEST_SYNC,
        DFU_STATE_dfuMANIFEST,
        DFU_STATE_dfuMANIFEST_WAIT_RESET,
        DFU_STATE_dfuUPLOAD_IDLE,
        DFU_STATE_dfuERROR
};

struct dfu_status_t {
        uint8_t bStatus;
        uint32_t bwPollTimeout : 24;
        uint8_t bState;
        uint8_t iString;
} __packed;
CTASSERT_SIZE_BYTE(struct dfu_status_t, 6);

const struct usb_desc_dev_t dfu_dev_desc = {
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
        .iProduct = 2,
        .iSerialNumber = 0,
        .bNumConfigurations = 1,
};

const struct {
        struct usb_desc_config_t config;
        struct usb_desc_iface_t iface;
        struct dfu_desc_function_t dfu;
} __packed dfu_config_desc = {
        .config = {
                .bLength = sizeof(struct usb_desc_config_t),
                .bDescriptorType = USB_DESC_CONFIG,
                .wTotalLength = sizeof(dfu_config_desc),
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
                .bInterfaceClass = USB_DEV_CLASS_APP,
                .bInterfaceSubClass = USB_DEV_SUBCLASS_APP_DFU,
                .bInterfaceProtocol = USB_DEV_PROTO_DFU,
                .iInterface = 0,
        },
        .dfu = {
                .bLength = sizeof(struct dfu_desc_function_t),
                .bDescriptorType = {
                        .id = 0x1,
                        .type = USB_DESC_TYPE_CLASS
                },
                .manifestation_tolerant = 1,
                .can_upload = 1,
                .can_download = 1,
                .wDetachTimeOut = 0,
                .wTransferSize = 4096,
                .bcdDFUVersion = { .maj = 1, .min = 1 }
        }
};

const struct usb_desc_string_t * const dfu_string_descs[] = {
        USB_DESC_STRING_LANG_ENUS,
        USB_DESC_STRING(u"mchck.org"),
        USB_DESC_STRING(u"MCHCK bootloader"),
        NULL
};

int
dfu_handle_control(struct usb_ctrl_req_t *req)
{
        switch ((enum dfu_ctrl_req_code)req->bRequest) {
        default:
                break;
        }
}
