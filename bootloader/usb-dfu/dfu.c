#include "usb.h"
#include "dfu.h"

enum dfu_dev_subclass {
        USB_DEV_SUBCLASS_APP_DFU = 0x01
};

enum dfu_dev_proto {
	USB_DEV_PROTO_DFU_APP = 0x01,
        USB_DEV_PROTO_DFU_DFU = 0x02
};

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

enum dfu_state {
        DFU_STATE_appIDLE = 0,
        DFU_STATE_appDETACH = 1,
        DFU_STATE_dfuIDLE = 2,
        DFU_STATE_dfuDNLOAD_SYNC = 3,
        DFU_STATE_dfuDNBUSY = 4,
        DFU_STATE_dfuDNLOAD_IDLE = 5,
        DFU_STATE_dfuMANIFEST_SYNC = 6,
        DFU_STATE_dfuMANIFEST = 7,
        DFU_STATE_dfuMANIFEST_WAIT_RESET = 8,
        DFU_STATE_dfuUPLOAD_IDLE = 9,
        DFU_STATE_dfuERROR = 10
};

struct dfu_status_t {
        enum dfu_status bStatus : 8;
        uint32_t bwPollTimeout : 24;
        enum dfu_state bState : 8;
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
                .bInterfaceProtocol = USB_DEV_PROTO_DFU_DFU,
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

struct dfu_dev_t {
        enum dfu_state state;
        enum dfu_status status;
        dfu_setup_write_t setup_write;
        dfu_finish_write_t finish_write;
        size_t off;
};

static struct dfu_dev_t dfu_dev;

void
dfu_write_done(enum dfu_status err)
{
        dfu_dev.status = err;
        if (dfu_dev.status == DFU_STATUS_OK)
                dfu_dev.state = DFU_STATE_dfuDNLOAD_SYNC;
        else
                dfu_dev.state = DFU_STATE_dfuERROR;
}

static void
dfu_dnload_complete(void *buf, ssize_t len, void *cbdata)
{
        dfu_dev.state = DFU_STATE_dfuDNBUSY;
        dfu_dev.status = dfu_dev.finish_write(dfu_dev.off, len);
        dfu_dev.off += len;

        if (dfu_dev.status == DFU_STATUS_OK) {
                usb_handle_control_status(NULL, 0, NULL);
        } else {
                dfu_dev.state = DFU_STATE_dfuERROR;
                usb_setup_control();
        }
}

int
dfu_handle_control(struct usb_ctrl_req_t *req)
{
        switch ((enum dfu_ctrl_req_code)req->bRequest) {
        case USB_CTRL_REQ_DFU_DNLOAD: {
                void *buf;

                switch (dfu_dev.state) {
                case DFU_STATE_dfuIDLE:
                        dfu_dev.off = 0;
                        break;
                case DFU_STATE_dfuDNLOAD_IDLE:
                        break;
                default:
                        goto err;
                }

                dfu_dev.status = dfu_dev.setup_write(dfu_dev.off, req->wLength, &buf);
                if (dfu_dev.status != DFU_STATUS_OK)
                        goto err;

                usb_rx(buf, req->wLength, dfu_dnload_complete, NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_GETSTATUS: {
                struct dfu_status_t st;

                switch (dfu_dev.state) {
                case DFU_STATE_dfuDNLOAD_SYNC:
                        dfu_dev.state = DFU_STATE_dfuDNLOAD_IDLE;
                        break;
                default:
                        break;
                }
                st.bState = dfu_dev.state;
                st.bStatus = dfu_dev.status;
                st.bwPollTimeout = 1000; /* XXX */
                usb_tx_cp(&st, sizeof(st), req->wLength, usb_handle_control_status, NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_CLRSTATUS:
                dfu_dev.state = DFU_STATE_dfuIDLE;
                usb_handle_control_status(NULL, 0, NULL);
                break;
        case USB_CTRL_REQ_DFU_GETSTATE: {
                uint8_t st = dfu_dev.state;
                usb_tx_cp(&st, sizeof(st), req->wLength, usb_handle_control_status, NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_ABORT:
                switch (dfu_dev.state) {
                case DFU_STATE_dfuIDLE:
                case DFU_STATE_dfuDNLOAD_IDLE:
                case DFU_STATE_dfuUPLOAD_IDLE:
                        dfu_dev.state = DFU_STATE_dfuIDLE;
                        break;
                default:
                        goto err;
                }
        case USB_CTRL_REQ_DFU_UPLOAD:
        default:
                dfu_dev.status = DFU_STATUS_errSTALLEDPKT;
                goto err;
        }

        return (1);

err:
        dfu_dev.state = DFU_STATE_dfuERROR;
        return (-1);
}

void
dfu_start(dfu_setup_write_t setup_write, dfu_finish_write_t finish_write)
{
        dfu_dev.state = DFU_STATE_dfuIDLE;
        dfu_dev.setup_write = setup_write;
        dfu_dev.finish_write = finish_write;
}
