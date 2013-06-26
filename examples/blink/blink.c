#include <mchck.h>

int
main(void)
{
	/* Enable PORTC clock */
	SIM_SCGC5 = 1 << SIM_SCGC5_PORTC_SHIFT;

	/* Configure pin as GPIO */
        PORTC.pcr[0].mux = PCR_MUX_GPIO;
        PORTC.pcr[0].dse = 1;

	/* Configure pin as output */
	GPIOC.pddr = 1 << 0;

	for (;;) {
		for (volatile int i = 1000000; i > 0; --i);
		GPIOC.ptor = 1 << 0;
	}
}
