#include <mchck.h>

/*
 * This code will increase the PWM duty cycle from 0% to 100%, then
 * decrease it from 100% to 0%, and so on.
 */
int
main(void)
{
	int inc = 1;		/* Currently incrementing */
	long fract v = 0.0;	/* Current PWM duty cycle */

	/* Initialize FTM and set PWM pin mode */
	ftm_init();
	pin_mode(PIN_PTC1, PIN_MODE_MUX_ALT4);

	/* XXX: Switch to timeouts instead of busy-wait */
	for (;;) {
		/* Increment or decrement PWM duty slightly */
		if (inc)
			v += 0.00001lr;
		else
			v -= 0.00001lr;

		/* Switch between increment and decrement */
		if (v >= 1 || v <= 0) inc = !inc;

		/* Set the actual PWM duty cycle */
		ftm_set(FTM_CH0, v);
	}
}
