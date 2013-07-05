#include <usb/usb.h>
#include <usb/dfu.h>

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


struct dfu_dev_t {
        enum dfu_state state;
        enum dfu_status status;
        dfu_setup_write_t setup_write;
        dfu_finish_write_t finish_write;
        size_t off;
        size_t len;
};


static struct dfu_dev_t dfu_dev;


void
dfu_write_done(enum dfu_status err)
{
        dfu_dev.status = err;
        if (dfu_dev.status == DFU_STATUS_OK) {
                switch (dfu_dev.state) {
                case DFU_STATE_dfuDNBUSY:
                        dfu_dev.state = DFU_STATE_dfuDNLOAD_IDLE;
                        break;
                default:
                        break;
                }
        } else {
                dfu_dev.state = DFU_STATE_dfuERROR;
        }
}

static void
dfu_dnload_complete(void *buf, ssize_t len, void *cbdata)
{
        if (len > 0)
                dfu_dev.state = DFU_STATE_dfuDNBUSY;
        else
                dfu_dev.state = DFU_STATE_dfuMANIFEST;
        dfu_dev.status = dfu_dev.finish_write(buf, dfu_dev.off, len);
        dfu_dev.off += len;
        dfu_dev.len = len;

        if (dfu_dev.status != DFU_STATUS_async)
                dfu_write_done(dfu_dev.status);

        usb_handle_control_status(dfu_dev.state == DFU_STATE_dfuERROR);
}

static void
dfu_reset_system(void *buf, ssize_t len, void *cbdata)
{
        sys_reset();
}

static void
dfu_handle_control(struct usb_ctrl_req_t *req)
{
        int fail = 1;

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

                /**
                 * XXX we are not allowed to STALL here, and we need to eat all transferred data.
                 * better not allow setup_write to break the protocol.
                 */
                dfu_dev.status = dfu_dev.setup_write(dfu_dev.off, req->wLength, &buf);
                if (dfu_dev.status != DFU_STATUS_OK) {
                        dfu_dev.state = DFU_STATE_dfuERROR;
                        goto err_have_status;
                }

                if (req->wLength > 0)
                        usb_ep0_rx(buf, req->wLength, dfu_dnload_complete, NULL);
                else
                        dfu_dnload_complete(NULL, 0, NULL);
                return;     /* We handle the status stage later */
        }
        case USB_CTRL_REQ_DFU_GETSTATUS: {
                struct dfu_status_t st;

                st.bState = dfu_dev.state;
                st.bStatus = dfu_dev.status;
                st.bwPollTimeout = 1000; /* XXX */
                /**
                 * If we're in DFU_STATE_dfuMANIFEST, we just finished
                 * the download, and we're just about to send our last
                 * status report.  Once the report has been sent, go
                 * and reset the system to put the new firmware into
                 * effect.
                 */
                usb_ep0_tx_cp(&st, sizeof(st), req->wLength,
                              dfu_dev.state == DFU_STATE_dfuMANIFEST ? dfu_reset_system : NULL,
                              NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_CLRSTATUS:
                dfu_dev.state = DFU_STATE_dfuIDLE;
                dfu_dev.status = DFU_STATUS_OK;
                break;
        case USB_CTRL_REQ_DFU_GETSTATE: {
                uint8_t st = dfu_dev.state;
                usb_ep0_tx_cp(&st, sizeof(st), req->wLength, NULL, NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_ABORT:
                switch (dfu_dev.state) {
                case DFU_STATE_dfuIDLE:
                case DFU_STATE_dfuDNLOAD_IDLE:
                /* case DFU_STATE_dfuUPLOAD_IDLE: */
                        dfu_dev.state = DFU_STATE_dfuIDLE;
                        break;
                default:
                        goto err;
                }
                break;
        /* case USB_CTRL_REQ_DFU_UPLOAD: */
        default:
                goto err;
        }

        fail = 0;
        goto out;

err:
        dfu_dev.status = DFU_STATUS_errSTALLEDPKT;
err_have_status:
        dfu_dev.state = DFU_STATE_dfuERROR;
out:
        usb_handle_control_status(fail);
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

const static struct {
        struct usb_desc_config_t config;
        struct usb_desc_iface_t iface;
        struct dfu_desc_function_t dfu;
} dfu_config_desc = {
        .config = {
                .bLength = sizeof(struct usb_desc_config_t),
                .bDescriptorType = USB_DESC_CONFIG,
                .wTotalLength = sizeof(dfu_config_desc),
                .bNumInterfaces = 1,
                .bConfigurationValue = 1,
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
                        .type_type = USB_DESC_TYPE_CLASS
                },
                .will_detach = 1,
                .manifestation_tolerant = 0,
                .can_upload = 1,
                .can_download = 1,
                .wDetachTimeOut = 0,
                .wTransferSize = USB_DFU_TRANSFER_SIZE,
                .bcdDFUVersion = { .maj = 1, .min = 1 }
        }
};

const struct usb_desc_string_t * const dfu_string_descs[] = {
        USB_DESC_STRING_LANG_ENUS,
        USB_DESC_STRING(u"mchck.org"),
        USB_DESC_STRING(u"MC HCK bootloader"),
        NULL
};

const struct usbd_identity_t dfu_identity = {
        .dev_desc = &dfu_dev_desc,
        .config_desc = &dfu_config_desc.config,
        .string_descs = dfu_string_descs,
        .class_control = dfu_handle_control
};


void
dfu_start(dfu_setup_write_t setup_write, dfu_finish_write_t finish_write)
{
        dfu_dev.state = DFU_STATE_dfuIDLE;
        dfu_dev.setup_write = setup_write;
        dfu_dev.finish_write = finish_write;
        usb_start(&dfu_identity);
}
