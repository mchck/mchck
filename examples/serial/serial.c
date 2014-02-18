#include <mchck.h>

struct timeout_ctx timeout;

static void
hello(void *cbdata)
{
        uart_write(UART_0, 'h');
        timeout_add(&timeout, 1000, hello, NULL);
}

void
main(void)
{
        pin_mode(PIN_PTA1, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTA2, PIN_MODE_MUX_ALT2);
        uart_init(UART_0);
        uart_set_baudrate(UART_0, 57600);
        uart_enable(UART_0);
        timeout_init();
        timeout_add(&timeout, 1000, hello, NULL);
        sys_yield_for_frogs();
}
