enum uart_id {
        UART_0,
        UART_1,
        UART_2
};

volatile struct UART_t *phys_uart_from_id(enum uart_id id);

void uart_init(enum uart_id id);

void uart_set_baudrate(enum uart_id id, unsigned int baudrate);

void uart_enable(enum uart_id id);
void uart_disable(enum uart_id id);

void uart_write(enum uart_id id, char c);
char uart_read(enum uart_id id);
