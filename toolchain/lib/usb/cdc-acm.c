#include <mchck.h>

#include <usb/usb.h>

#include "cdc-acm.h"


static void
cdc_rx_done(void *buf, ssize_t len, void *data)
{
        struct cdc_ctx *ctx = data;

        if (ctx->data_ready_cb)
                ctx->data_ready_cb(buf, len);
}

void
cdc_read_more(struct cdc_ctx *ctx)
{
        usb_rx(ctx->rx_pipe, ctx->inbuf, sizeof(ctx->inbuf), cdc_rx_done, ctx);
}

static void
cdc_tx_done(void *buf, ssize_t len, void *data)
{
        struct cdc_ctx *ctx = data;

        if (len <= 0)
                goto queue;

        crit_enter();
        ctx->out_sent += len;
        memcpy(ctx->outbuf, &ctx->outbuf[ctx->out_sent], ctx->out_pos - ctx->out_sent);
        ctx->out_pos -= ctx->out_sent;
        ctx->out_sent = 0;
        ctx->out_queued = 0;
        crit_exit();
        if (ctx->data_sent_cb != NULL)
                ctx->data_sent_cb(sizeof(ctx->outbuf) - ctx->out_pos);
queue:
        crit_enter();
        if (ctx->out_pos > 0 && !ctx->out_queued) {
                ctx->out_queued = 1;
                usb_tx(ctx->tx_pipe, ctx->outbuf, ctx->out_pos, CDC_TX_SIZE, cdc_tx_done, ctx);
        }
        crit_exit();
}

size_t
cdc_write_space(struct cdc_ctx *ctx)
{
        return (sizeof(ctx->outbuf) - ctx->out_pos);
}

ssize_t
cdc_write(const uint8_t *buf, size_t len, struct cdc_ctx *ctx)
{
        size_t max_len;

        crit_enter();
        max_len = cdc_write_space(ctx);
        if (len > max_len)
                len = max_len;

        memcpy(&ctx->outbuf[ctx->out_pos], buf, len);
        ctx->out_pos += len;

        cdc_tx_done(ctx->outbuf, -1, ctx);
        crit_exit();

        return (len);
}

ssize_t
cdc_write_string(const char *s, struct cdc_ctx *ctx)
{
        return (cdc_write((const void *)s, strlen(s), ctx));
}

/**
 * control requests received:
 * 0x21, 0x34
 * 0x21, 0x32
 */


const struct usbd_function cdc_function = {
        .interface_count = USB_FUNCTION_CDC_IFACE_COUNT,
};

void
cdc_init(void (*data_ready_cb)(uint8_t *, size_t), void (*data_sent_cb)(size_t), struct cdc_ctx *ctx)
{
        usb_attach_function(&cdc_function, &ctx->header);
        ctx->data_ready_cb = data_ready_cb;
        ctx->data_sent_cb = data_sent_cb;
        ctx->out_queued = 0;
        ctx->notice_pipe = usb_init_ep(&ctx->header, CDC_NOTICE_EP, USB_EP_TX, CDC_NOTICE_SIZE);
	ctx->tx_pipe = usb_init_ep(&ctx->header, CDC_TX_EP, USB_EP_TX, CDC_TX_SIZE);
	ctx->rx_pipe = usb_init_ep(&ctx->header, CDC_RX_EP, USB_EP_RX, CDC_RX_SIZE);
        cdc_read_more(ctx);
        cdc_tx_done(ctx->outbuf, -1, ctx);
}


#ifndef VUSB
static size_t
cdc_stdio_write(const uint8_t *buf, size_t len, void *data)
{
        return (cdc_write(buf, len, data));
}

static const struct _stdio_file_ops cdc_ops = {
        .write = cdc_stdio_write,
};

void
cdc_set_stdout(struct cdc_ctx *cdc)
{
        stdout->ops_data = cdc;
        stdout->ops = &cdc_ops;
}
#endif
