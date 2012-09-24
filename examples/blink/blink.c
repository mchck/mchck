#include <mchck.h>

int
main(void)
{
	/* Enable PORTC clock */
	SIM_SCGC5 = 1 << SIM_SCGC5_PORTC_SHIFT;

	/* Configure pin as GPIO */
	PORTC_PCR0 = PORT_PCR_MUX(1) | (1 << PORT_PCR_DSE_SHIFT);

	/* Configure pin as output */
	GPIOC_PDDR = 1 << 0;

	for (;;) {
		GPIOC_PTOR = 1 << 0;
		for (volatile int i = 1000000; i > 0; --i);
	}
}
