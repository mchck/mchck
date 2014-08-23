#include <mchck.h>

char test[] = "hello world! ";
char read_test[10];

struct uart_trans_ctx rx_trans, tx_trans, tx_trans2;

static void
read_done(const void *buf, size_t len, void *cbdata)
{
        onboard_led(ONBOARD_LED_TOGGLE);
        uart_write(&uart0, &tx_trans, test, sizeof(test), NULL, NULL);
        uart_write(&uart0, &tx_trans2, read_test, sizeof(read_test), NULL, NULL);
        uart_read(&uart0, &rx_trans, read_test, sizeof(read_test), read_done, NULL);
}

void
main(void)
{
        pin_mode(PIN_PTA1, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTA2, PIN_MODE_MUX_ALT2);
        uart_init(&uart0);
        uart_set_baudrate(&uart0, 57600);
        read_done(NULL, 0, NULL);
        sys_yield_for_frogs();
}
