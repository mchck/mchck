#include <mchck.h>

static const enum gpio_pin_id led_pin = GPIO_PTC0;
static struct timeout_ctx t;

static void
blink(void *data)
{
	timeout_add(&t, 500, blink, NULL);
	gpio_toggle(led_pin);
}

int
main(void)
{
	gpio_dir(led_pin, GPIO_OUTPUT);
	pin_mode(led_pin, PIN_MODE_DRIVE_HIGH);

	timeout_init();
	/* blink will also setup a timer to itself */
	blink(NULL);
	sys_yield_for_frogs();
}
