typedef void (*uart_cb)(void *cbdata);

struct uart_ctx {
        volatile struct UART_t *uart;
        struct uart_trans_ctx *tx_queue;
        struct uart_trans_ctx *rx_queue;
};

struct uart_trans_ctx {
        char *pos;
        unsigned int remaining;
        struct uart_trans_flags {
                int stop_on_terminator : 1;
        } flags;
        char terminator;
        uart_cb cb;
        void *cbdata;
        struct uart_trans_ctx *next;
};

extern struct uart_ctx uart0, uart1, uart2;

void uart_init(struct uart_ctx *uart);

void uart_set_baudrate(struct uart_ctx *uart, unsigned int baudrate);

void uart_enable(struct uart_ctx *uart);
void uart_disable(struct uart_ctx *uart);

void uart_write(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
                const char *c, size_t len,
                uart_cb cb, void *cbdata);
void uart_read(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
               char *c, size_t len,
               uart_cb cb, void *cbdata);
void uart_read_until(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
                     char *c, size_t len, char until,
                     uart_cb cb, void *cbdata);
