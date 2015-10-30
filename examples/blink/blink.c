#include <mchck.h>

static struct timeout_ctx t;

static void
blink(void *data)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	timeout_add(&t, 500, blink, NULL);
}

int
main(void)
{
	timeout_init();
	/* blink will also setup a timer to itself */
	blink(NULL);
	sys_yield_for_frogs();
}
