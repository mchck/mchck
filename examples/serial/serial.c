#include <mchck.h>

struct timeout_ctx timeout;

char test[] = "hello world!";

struct uart_trans_ctx trans;

static void
hello(void *cbdata)
{
        uart_write(&uart0, &trans, test, sizeof(test), NULL, NULL);
        timeout_add(&timeout, 1000, hello, NULL);
}

void
main(void)
{
        pin_mode(PIN_PTA1, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTA2, PIN_MODE_MUX_ALT2);
        uart_init(&uart0);
        uart_set_baudrate(&uart0, 57600);
        uart_enable(&uart0);
        timeout_init();
        timeout_add(&timeout, 1000, hello, NULL);
        sys_yield_for_frogs();
}
