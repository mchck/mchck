#include <mchck.h>

struct dfu_app_ctx {
        struct usbd_function_ctx_header header;
        dfu_detach_t detachcb;
};

static struct dfu_app_ctx dfu_app_ctx;


static void
dfu_app_detach(void *buf, ssize_t len, void *data)
{
        if (dfu_app_ctx.detachcb)
                dfu_app_ctx.detachcb();
}

static int
dfu_app_handle_control(struct usb_ctrl_req_t *req, void *data)
{
        switch ((enum dfu_ctrl_req_code)req->bRequest) {
        case USB_CTRL_REQ_DFU_DETACH:
                usb_handle_control_status_cb(dfu_app_detach);
                return (1);

        /**
         * The following two requests are optional, but dfu-util
         * requires at least DFU_GETSTATUS.
         */
        case USB_CTRL_REQ_DFU_GETSTATUS: {
                struct dfu_status_t st;

                st.bState = DFU_STATE_appIDLE;;
                st.bStatus = DFU_STATUS_OK;
                st.bwPollTimeout = 0;
                usb_ep0_tx_cp(&st, sizeof(st), req->wLength, NULL, NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_GETSTATE: {
                uint8_t st = DFU_STATUS_OK;
                usb_ep0_tx_cp(&st, sizeof(st), req->wLength, NULL, NULL);
                break;
        }
        default:
                return (0);
        }

        usb_handle_control_status(0);
        return (1);
}


void
dfu_app_init(dfu_detach_t detachcb)
{
        dfu_app_ctx.detachcb = detachcb;
        usb_attach_function(&dfu_app_function, &dfu_app_ctx.header);
}

const struct usbd_function dfu_app_function = {
        .control = dfu_app_handle_control,
        .interface_count = USB_FUNCTION_DFU_IFACE_COUNT,
};
