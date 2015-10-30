#include <usb/usb.h>
#include <usb/dfu.h>


void
dfu_write_done(enum dfu_status err, struct dfu_ctx *ctx)
{
        ctx->status = err;
        if (ctx->status != DFU_STATUS_OK)
                ctx->state = DFU_STATE_dfuERROR;

        switch (ctx->state) {
        case DFU_STATE_dfuDNLOAD_SYNC:
                ctx->state = DFU_STATE_dfuDNLOAD_IDLE;
                break;
        case DFU_STATE_dfuMANIFEST_SYNC:
                ctx->state = DFU_STATE_dfuIDLE;
                break;
        default:
                break;
        }
}

static void
dfu_dnload_complete(void *buf, ssize_t len, void *cbdata)
{
        struct dfu_ctx *ctx = cbdata;

        enum dfu_status s = ctx->finish_write(buf, ctx->off, len);
        ctx->off += len;
        ctx->len = len;

        if (s != DFU_STATUS_async)
                dfu_write_done(s, ctx);

        usb_handle_control_status(ctx->state == DFU_STATE_dfuERROR);
}

static int
dfu_handle_control(struct usb_ctrl_req_t *req, void *data)
{
        struct dfu_ctx *ctx = data;
        int fail = 1;

        switch ((enum dfu_ctrl_req_code)req->bRequest) {
        case USB_CTRL_REQ_DFU_DNLOAD: {
                void *buf;

                switch (ctx->state) {
                case DFU_STATE_dfuIDLE:
                        ctx->off = 0;
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
                ctx->status = ctx->setup_write(ctx->off, req->wLength, &buf);
                if (ctx->status != DFU_STATUS_OK) {
                        ctx->state = DFU_STATE_dfuERROR;
                        goto err_have_status;
                }

                if (req->wLength > 0) {
                        ctx->state = DFU_STATE_dfuDNLOAD_SYNC;
                        usb_ep0_rx(buf, req->wLength, dfu_dnload_complete, ctx);
                } else {
                        ctx->state = DFU_STATE_dfuMANIFEST_SYNC;
                        dfu_dnload_complete(NULL, 0, ctx);
                }
                goto out_no_status;
        }
        case USB_CTRL_REQ_DFU_GETSTATUS: {
                struct dfu_status_t st;

                st.bState = ctx->state;
                st.bStatus = ctx->status;
                st.bwPollTimeout = 1; /* XXX allow setting in desc? */
                /**
                 * If we're in DFU_STATE_dfuMANIFEST, we just finished
                 * the download, and we're just about to send our last
                 * status report.  Once the report has been sent, go
                 * and reset the system to put the new firmware into
                 * effect.
                 */
                usb_ep0_tx_cp(&st, sizeof(st), req->wLength, NULL, NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_CLRSTATUS:
                if (ctx->state != DFU_STATE_dfuERROR)
                        goto err;
                ctx->state = DFU_STATE_dfuIDLE;
                ctx->status = DFU_STATUS_OK;
                break;
        case USB_CTRL_REQ_DFU_GETSTATE: {
                uint8_t st = ctx->state;
                usb_ep0_tx_cp(&st, sizeof(st), req->wLength, NULL, NULL);
                break;
        }
        case USB_CTRL_REQ_DFU_ABORT:
                switch (ctx->state) {
                case DFU_STATE_dfuIDLE:
                case DFU_STATE_dfuDNLOAD_IDLE:
                /* case DFU_STATE_dfuUPLOAD_IDLE: */
                        ctx->state = DFU_STATE_dfuIDLE;
                        break;
                default:
                        goto err;
                }
                break;
        /* case USB_CTRL_REQ_DFU_UPLOAD: */
        default:
                return (0);
        }

        fail = 0;
        goto out;

err:
        ctx->status = DFU_STATUS_errSTALLEDPKT;
err_have_status:
        ctx->state = DFU_STATE_dfuERROR;
out:
        usb_handle_control_status(fail);
out_no_status:
        return (1);
}

void
dfu_init(dfu_setup_write_t setup_write, dfu_finish_write_t finish_write, struct dfu_ctx *ctx)
{
        ctx->state = DFU_STATE_dfuIDLE;
        ctx->setup_write = setup_write;
        ctx->finish_write = finish_write;
        usb_attach_function(&dfu_function, &ctx->header);
}

const struct usbd_function dfu_function = {
        .control = dfu_handle_control,
        .interface_count = USB_FUNCTION_DFU_IFACE_COUNT,
};
