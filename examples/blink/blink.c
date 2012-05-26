#include <mchck.h>
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"

int
main(void)
{
	/* The LED is on PC13 */

        static GPIO_InitTypeDef pininit = {
		.GPIO_Pin = 1 << 13,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_Speed = GPIO_Speed_400KHz,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL
	};

	/* Enable GPIOC clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	/* Configure pin as output */
	GPIO_Init(GPIOC, &pininit);

	for (;;) {
		GPIO_ToggleBits(GPIOC, 1 << 13);
		for (volatile int i = 1000000; i > 0; --i);
	}
}
