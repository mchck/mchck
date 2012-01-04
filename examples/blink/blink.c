#include "NUC1xx.h"

int
main(void)
{
        /* LED is on PA5 */
        GPIOA->PMD.PMD5 = 1;    /* output mode */

	for (;;) {
		GPIOA->DOUT ^= 1 << 5;
		for (volatile int i = 1000000; i > 0; --i);
	}
}
