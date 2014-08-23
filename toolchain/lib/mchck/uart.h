typedef void (*uart_cb)(const void *buf, size_t len, void *cbdata);

struct uart_ctx {
        volatile struct UART_t *uart;
        struct uart_trans_ctx *tx_queue;
        struct uart_trans_ctx *rx_queue;
};

struct uart_trans_ctx {
        uint8_t *buf;
        uint8_t *pos;
        unsigned int remaining;
        struct uart_trans_flags {
                int stop_on_terminator : 1;
        } flags;
        uint8_t terminator;
        uart_cb cb;
        void *cbdata;
        struct uart_trans_ctx **queue;
        struct uart_trans_ctx *next;
};

extern struct uart_ctx uart0, uart1, uart2;

void uart_init(struct uart_ctx *uart);

void uart_set_baudrate(struct uart_ctx *uart, unsigned int baudrate);

int uart_write(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
               const void *buf, size_t len,
               uart_cb cb, void *cbdata);
void uart_read(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
               void *buf, size_t len,
               uart_cb cb, void *cbdata);
void uart_read_until(struct uart_ctx *uart, struct uart_trans_ctx *ctx,
                     void *buf, size_t len, char until,
                     uart_cb cb, void *cbdata);
int uart_abort(struct uart_ctx *uart, struct uart_trans_ctx *ctx);
