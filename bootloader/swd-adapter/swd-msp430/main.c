#include <msp430.h>
#include <legacymsp430.h>
#include <stdlib.h>
#include <signal.h>

#include "../swduino/swd.h"


unsigned char tx_buffer[16];
volatile unsigned int tail = 0, head = 0;

void
signal_led(void)
{
        P1OUT ^= BIT0;
}

int
pin_read(enum swd_pin pin)
{
        return !!(P1IN & pin);
}

int
outpipe_space(size_t len)
{
        return 1; // XXX can always write (I hope)
}

void
pin_write(enum swd_pin pin, int val)
{
        if (val) {
                P1OUT |= pin;
        }else{
                P1OUT &= ~pin;
        }
}

void
reply_write(const uint8_t *buf, size_t len)
{
        int i = 0;

        for(; i < len; i++) {
                unsigned int pos = (head+1) % 16;

                while(pos == tail)//buffer is full, wait
                        /* NOTHING */;

                tx_buffer[head] = buf[i];
                head = pos;

                IE2 |= UCA0TXIE;//enable interrupt                  		
        }
}

void
pin_configure(enum swd_pin pin, enum swd_pin_mode mode)
{
        if (mode == SWD_MODE_OUTPUT) {
                P1DIR |= pin;
        }else{
                P1DIR &= ~pin;
        }
}

interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
        uint8_t x = UCA0RXBUF;
        process_data(&x, 1);
}

interrupt(USCIAB0TX_VECTOR) USCI0TX_ISR(void)
{
	if (head == tail) {
		//buffer empty

		IE2 &= ~UCA0TXIE;//disable interrupt
		return;
	}
	
	unsigned char c = tx_buffer[tail];
	tail = (tail+1)%16;
	
	UCA0TXBUF = c;
}

int
main()
{
        WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
        P1DIR |= BIT0; //set BIT0 as output = LED

        BCSCTL1 = CALBC1_16MHZ;
        DCOCTL = CALDCO_16MHZ;

        __bis_SR_register(GIE);

        //UART
        P1SEL  = BIT1 + BIT2;                       
        P1SEL2 = BIT1 + BIT2;                       
        UCA0CTL1 |= UCSSEL_2; // SMCLK

        //use this for 9600 bps
        UCA0BR0 = 130;                          // 16MHz 9600
        UCA0BR1 = 6;                              // 16MHz 9600

        UCA0MCTL = UCBRS1 + UCBRS0; // Modulation UCBRSx = 3
        UCA0CTL1 &= ~UCSWRST;
        IE2 |= UCA0RXIE; //enable interrupts

        for (;;)
                /* NOTHING */;
}
