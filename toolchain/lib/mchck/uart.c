#include <mchck.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

struct uart_ctx uart0 = {.uart = &UART0};
struct uart_ctx uart1 = {.uart = &UART1};
struct uart_ctx uart2 = {.uart = &UART2};

void
uart_init(struct uart_ctx *uart)
{
        if (uart->uart == &UART0) {
                SIM.scgc4.uart0 = 1;
        } else if (uart->uart == &UART1) {
                SIM.scgc4.uart1 = 1;
        } else if (uart->uart == &UART2) {
                SIM.scgc4.uart2 = 1;
        }

        // Enable FIFOs
        uart->uart->pfifo.rxfe = 1;
        uart->uart->pfifo.txfe = 1;
        // XXX arbitrary FIFO watermarks
        uart->uart->twfifo = 8;
        uart->uart->rwfifo = 1; // FIXME: See comment at end of uart_start_rx

        if (uart->uart == &UART0) {
                int_enable(IRQ_UART0_status);
        } else if (uart->uart == &UART1) {
                int_enable(IRQ_UART1_status);
        } else if (uart->uart == &UART2) {
                int_enable(IRQ_UART2_status);
        }
}

void
uart_set_baudrate(struct uart_ctx *uart, unsigned int baudrate)
{
        unsigned int clockrate = 48000000; /* XXX use real clock rate */
        unsigned int sbr = clockrate / 16 / baudrate;
        unsigned int brfa = (2 * clockrate / baudrate) % 32;
        uart->uart->bdh.sbrh = sbr >> 8;
        uart->uart->bdl.sbrl = sbr & 0xff;
        uart->uart->c4.brfa = brfa;
}

static void
uart_queue_transfer(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
                    void (*start_queue)(struct uart_ctx *uart))
{
        crit_enter();
        ctx->next = NULL;
        for (struct uart_trans_ctx **c = ctx->queue; ; c = &(*c)->next) {
                if (*c == NULL) {
                        *c = ctx;
                        /* we're at the head, so start transfer */
                        if (c == ctx->queue)
                                start_queue(uart);
                        break;
                }
        }
        crit_exit();
}

static void
uart_start_tx(struct uart_ctx *uart)
{
        uart->uart->c2.te = 1;

        unsigned int depth = 1 << (uart->uart->pfifo.txfifosize + 1);
        if (depth == 2)
                depth = 1;

        while (uart->uart->tcfifo < depth) {
                struct uart_trans_ctx *ctx = uart->tx_queue;
                if (!ctx)
                        return;
                uart->uart->d = *ctx->pos;
                ctx->pos++;
                ctx->remaining--;
                if (ctx->remaining == 0) {
                        uart->tx_queue = ctx->next;
                        if (ctx->cb)
                                ctx->cb(ctx->buf, ctx->pos - ctx->buf, ctx->cbdata);
                }
        }
}

int
uart_write(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
           const void *buf, size_t len,
           uart_cb cb, void *cbdata)
{
        if (ctx->remaining)
                return -1;
        ctx->pos = ctx->buf = (void *)buf;
        ctx->remaining = len;
        ctx->cb = cb;
        ctx->cbdata = cbdata;
        ctx->queue = &uart->tx_queue;
        uart->uart->c2.tie = 1;
        uart_queue_transfer(uart, ctx, uart_start_tx);
        return 0;
}

static void
uart_start_rx(struct uart_ctx *uart)
{
        int remaining;

        uart->uart->c2.re = 1;
        while ((remaining = uart->uart->rcfifo) != 0) {
                struct uart_trans_ctx *ctx = uart->rx_queue;
                if (!ctx)
                        return;
                /* clear flags */
                if (remaining == 1)
                        (void)uart->uart->s1;
                *ctx->pos = uart->uart->d;
                ctx->pos++;
                ctx->remaining--;

                bool stop = false;
                stop |= ctx->flags.stop_on_terminator
                        && *(ctx->pos-1) == ctx->terminator;
                stop |= ctx->remaining == 0;
                if (stop) {
                        uart->rx_queue = ctx->next;
                        ctx->remaining = 0;
                        if (ctx->cb)
                                ctx->cb(ctx->buf, ctx->pos - ctx->buf, ctx->cbdata);
                }
        }

        /*
         * ensure watermark is low enough.
         * otherwise we may not be notified when the receive completes.
         */
        #if 0
        /* FIXME: This seems to lock up the peripheral */
        unsigned int depth = 1 << (uart->uart->pfifo.rxfifosize + 1);
        uart->uart->c2.re = 0;
        uart->uart->rwfifo = MIN(uart->uart->pfifo.rxfifosize - 1,
                                 uart->rx_queue->remaining);
        uart->uart->c2.re = 1;
        #endif
}

void
uart_read(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
          void *buf, size_t len,
          uart_cb cb, void *cbdata)
{
        if (ctx->remaining)
                return;
        ctx->flags.stop_on_terminator = false;
        ctx->pos = ctx->buf = buf;
        ctx->remaining = len;
        ctx->cb = cb;
        ctx->cbdata = cbdata;
        ctx->queue = &uart->rx_queue;
        uart->uart->c2.rie = 1;
        uart_queue_transfer(uart, ctx, uart_start_rx);
}

void
uart_read_until(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
                void *buf, size_t len, char until,
                uart_cb cb, void *cbdata)
{
        if (ctx->remaining)
                return;
        ctx->flags.stop_on_terminator = true;
        ctx->terminator = until;
        ctx->pos = ctx->buf = buf;
        ctx->remaining = len;
        ctx->cb = cb;
        ctx->cbdata = cbdata;
        ctx->queue = &uart->rx_queue;
        uart->uart->c2.rie = 1;
        uart_queue_transfer(uart, ctx, uart_start_rx);
}

int
uart_abort(struct uart_ctx *uart, struct uart_trans_ctx *ctx)
{
        int success = 0;

        crit_enter();
        for (struct uart_trans_ctx **c = ctx->queue; *c != NULL; c = &(*c)->next) {
                if (*c == ctx) {
                        *c = ctx->next;
                        success = 1;
                        ctx->remaining = 0;
                        if (ctx->pos != ctx->buf && ctx->cb)
                                ctx->cb(ctx->buf, ctx->pos - ctx->buf, ctx->cbdata);
                        break;
                }
        }
        crit_exit();

        return (success);
}

void
uart_irq_handler(struct uart_ctx *uart)
{
        struct UART_S1_t s1 = { .raw = uart->uart->s1.raw };

        if (s1.tdre) {
                if (uart->tx_queue != NULL)
                        uart_start_tx(uart);
                else
                        uart->uart->c2.tie = 0;
        }
        if (uart->uart->rcfifo > 0) {
                if (uart->rx_queue != NULL)
                        uart_start_rx(uart);
                else
                        uart->uart->c2.rie = 0;
        }
        if (s1.fe) {
                /* simply clear flag and hope for the best */
                (void) uart->uart->d;
        }

        /* final clear of watermark flags */
        (void)uart->uart->s1.raw;
}

void
UART0_status_Handler(void)
{
        uart_irq_handler(&uart0);
}

void
UART1_status_Handler(void)
{
        uart_irq_handler(&uart1);
}

void
UART2_status_Handler(void)
{
        uart_irq_handler(&uart2);
}
