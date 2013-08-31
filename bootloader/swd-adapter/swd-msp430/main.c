#include <msp430.h>
#include <legacymsp430.h>
#include <stdlib.h>
#include <signal.h>

#include "../swduino/swd.h"

struct buffer {
        uint8_t buf[16];
        volatile unsigned int tail;
        volatile unsigned int head;
};

static struct buffer tx_buffer = { .buf = {0}, .tail = 0, .head = 0 };

void
signal_led(void)
{
        P1OUT ^= BIT0;
}

int
pin_read(enum swd_pin pin)
{
        return (P1IN & pin);
}

int
outpipe_space(size_t len)
{
        uint8_t available;

        if (tx_buffer.tail > tx_buffer.head)
                available = tx_buffer.tail - tx_buffer.head;
        else
                available = sizeof(tx_buffer.buf) - tx_buffer.head + tx_buffer.tail;

        if (available >= len)
                return (1);

        return (0);
}

void
pin_write(enum swd_pin pin, int val)
{
        if (val)
                P1OUT |= pin;
        else
                P1OUT &= ~pin;
}

void
reply_write(const uint8_t *buf, size_t len)
{
        int i = 0;

        for(; i < len; i++) {
                unsigned int pos = (tx_buffer.head + 1) % sizeof(tx_buffer.buf);

                /* buffer is full, wait */
                while (pos == tx_buffer.tail)
                        /* NOTHING */;

                tx_buffer.buf[tx_buffer.head] = buf[i];
                tx_buffer.head = pos;

                IE2 |= UCA0TXIE; /* enable interrupt */
        }
}

void
pin_configure(enum swd_pin pin, enum swd_pin_mode mode)
{
        if (mode == SWD_MODE_OUTPUT)
                P1DIR |= pin;
        else
                P1DIR &= ~pin;
}

interrupt(USCIAB0RX_VECTOR)
USCI0RX_ISR(void)
{
        uint8_t x = UCA0RXBUF;
        process_data(&x, 1);
}

interrupt(USCIAB0TX_VECTOR)
USCI0TX_ISR(void)
{
        if (tx_buffer.head == tx_buffer.tail) {
                /* buffer empty */

                IE2 &= ~UCA0TXIE; /* disable interrupt */
                return;
        }

        uint8_t c = tx_buffer.buf[tx_buffer.tail];
        tx_buffer.tail = (tx_buffer.tail + 1) % sizeof(tx_buffer.buf);

        UCA0TXBUF = c;
}

int
main(void)
{
        WDTCTL = WDTPW + WDTHOLD; /* Stop watchdog timer */
        P1DIR |= BIT0;            /* set BIT0 as output = LED */

        BCSCTL1 = CALBC1_16MHZ;
        DCOCTL = CALDCO_16MHZ;

        __bis_SR_register(GIE);

        /* UART */
        P1SEL  = BIT1 + BIT2;
        P1SEL2 = BIT1 + BIT2;
        UCA0CTL1 |= UCSSEL_2;   /* SMCLK */

        /* use this for 16MHz/9600 bps */
        UCA0BR0 = 130;
        UCA0BR1 = 6;

        UCA0MCTL = UCBRS1 + UCBRS0; /* Modulation UCBRSx = 3 */
        UCA0CTL1 &= ~UCSWRST;
        IE2 |= UCA0RXIE;        /* enable interrupts */

        for (;;)
                /* NOTHING */;
}
