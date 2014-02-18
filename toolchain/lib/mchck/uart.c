#include <mchck.h>

volatile struct UART_t *
phys_uart_from_id(enum uart_id id)
{
        switch (id) {
        case UART_0:
                return (&UART0);
        case UART_1:
                return (&UART1);
        case UART_2:
                return (&UART2);
        default:
                return (NULL);
        }
}

void
uart_init(enum uart_id id)
{
        volatile struct UART_t *uart = phys_uart_from_id(id);
        switch (id) {
        case UART_0:
                SIM.scgc4.uart0 = 1;
                break;
        case UART_1:
                SIM.scgc4.uart1 = 1;
                break;
        case UART_2:
                SIM.scgc4.uart2 = 1;
                break;
        }

        // Enable FIFOs
        uart->pfifo.rxfe = 1;
        uart->pfifo.txfe = 1;
}

void
uart_set_baudrate(enum uart_id id, unsigned int baudrate)
{
        unsigned int clockrate = 24000000;
        unsigned int sbr = clockrate / 16 / baudrate;
        unsigned int brfa = (2 * clockrate / baudrate) % 32;
        volatile struct UART_t *uart = phys_uart_from_id(id);
        uart->bdh.sbrh = sbr >> 8;
        uart->bdl.sbrl = sbr & 0xff;
        uart->c4.brfa = brfa;
}

void
uart_enable(enum uart_id id)
{
        phys_uart_from_id(id)->c2.raw |= ((struct UART_C2_t) {.re = 1, .te = 1}.raw);
}

void
uart_disable(enum uart_id id)
{
        phys_uart_from_id(id)->c2.raw &= ~ ((struct UART_C2_t) {.re = 1, .te = 1}.raw);
}

void
uart_write(enum uart_id id, char c)
{
        phys_uart_from_id(id)->d = c;
}

char
uart_read(enum uart_id id)
{
        return phys_uart_from_id(id)->d;
}
