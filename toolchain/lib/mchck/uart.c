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
        unsigned int clockrate = 48000000;
        unsigned int sbr = clockrate / 16 / baudrate;
        unsigned int brfa = (2 * clockrate / baudrate) % 32;
        uart->uart->bdh.sbrh = sbr >> 8;
        uart->uart->bdl.sbrl = sbr & 0xff;
        uart->uart->c4.brfa = brfa;
}

static void
uart_queue_transfer(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
                    struct uart_trans_ctx **queue,
                    void (*start_queue)(struct uart_ctx *uart))
{
        crit_enter();
        ctx->next = NULL;
        for (struct uart_trans_ctx **c = queue; ; c = &(*c)->next) {
                if (*c == NULL) {
                        *c = ctx;
                        /* we're at the head, so start transfer */
                        if (c == queue)
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
        while (uart->uart->tcfifo < depth) {
                struct uart_trans_ctx *ctx = uart->tx_queue;
                if (!ctx) {
                        uart->uart->c2.tie = 0;
                        uart->uart->c2.te = 0;
                        return;
                }
                uart->uart->d = *ctx->pos;
                ctx->pos++;
                ctx->remaining--;
                if (ctx->remaining == 0) {
                        uart->tx_queue = ctx->next;
                        if (ctx->cb)
                                ctx->cb(ctx->cbdata);
                }
        }
}

void
uart_write(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
           const char *c, size_t len,
           uart_cb cb, void *cbdata)
{
        if (ctx->remaining)
                return;
        ctx->pos = (char *) c;
        ctx->remaining = len;
        ctx->cb = cb;
        ctx->cbdata = cbdata;
        uart->uart->c2.tie = 1;
        uart_queue_transfer(uart, ctx, &uart->tx_queue, uart_start_tx);
}

static void
uart_start_rx(struct uart_ctx *uart)
{
        uart->uart->c2.re = 1;
        while (uart->uart->rcfifo > 0) {
                struct uart_trans_ctx *ctx = uart->rx_queue;
                if (!ctx) {
                        uart->uart->c2.rie = 0;
                        uart->uart->c2.re = 0;
                        return;
                }
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
                                ctx->cb(ctx->cbdata);
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
          char *c, size_t len,
          uart_cb cb, void *cbdata)
{
        if (ctx->remaining)
                return;
        ctx->flags.stop_on_terminator = false;
        ctx->pos = c;
        ctx->remaining = len;
        ctx->cb = cb;
        ctx->cbdata = cbdata;
        uart->uart->c2.rie = 1;
        uart_queue_transfer(uart, ctx, &uart->rx_queue, uart_start_rx);
}

void
uart_read_until(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
                char *c, size_t len, char until,
                uart_cb cb, void *cbdata)
{
        if (ctx->remaining)
                return;
        ctx->flags.stop_on_terminator = true;
        ctx->terminator = until;
        ctx->pos = c;
        ctx->remaining = len;
        ctx->cb = cb;
        ctx->cbdata = cbdata;
        uart->uart->c2.rie = 1;
        uart_queue_transfer(uart, ctx, &uart->rx_queue, uart_start_rx);
}

void
uart_irq_handler(struct uart_ctx *uart)
{
        if (uart->uart->s1.tdre) {
                uart_start_tx(uart);
        }
        if (uart->uart->s1.rdrf) {
                uart_start_rx(uart);
        }
        if (uart->uart->s1.fe) {
                /* simply clear flag and hope for the best */
                (void) uart->uart->d;
        }

        /* final clear of watermark flags */
        (void)uart->uart->s1.rdrf;
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
