#include <mchck.h>

int
main(void)
{
	for (;;) {
		for (volatile int i = 1000000; i > 0; --i)
			/* NOTHING */;
		onboard_led(ONBOARD_LED_TOGGLE);
	}
}
