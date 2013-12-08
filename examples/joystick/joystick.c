/*
 * Support for nice little joystick available on eBay. Search for JoyStick Breakout Module Shield
 * http://bit.ly/1ghTSNx
 *
 * Outputs a stream of lines on USB serial.
 *
 * k0 - key is down
 * k1 - key is up
 * xn - movement in X direction, where n is +/- JOYSTICK_CENTER
 * yn - movement in Y direction, where n is +/- JOYSTICK_CENTER
 *
 * Key output occurs on press or release, x and y output repeatedly when not centered.
 */

#include <mchck.h>

#include "pin_cb.h"
#include "joystick.desc.h"

static struct cdc_ctx cdc;
static struct timeout_ctx t;

const enum adc_channel x_pin = ADC_PTD6;
const enum adc_channel y_pin = ADC_PTD5;
const enum gpio_pin_id key_pin = GPIO_PTD7;

void read_input(void *cbdata);

struct input {
	const enum adc_channel channel;
	const char *tag;
} inputs[] = {
	{ ADC_PTD6, "x" },
	{ ADC_PTD5, "y" }
};

#define NUM_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))

#define JOYSTICK_RANGE 9
#define JOYSTICK_CENTER (JOYSTICK_RANGE / 2)
#define JOYSTICK_DELAY	20
#define MAX_UINT16	(uint16_t)65535

void read_cb(uint16_t val, int error, void *cbdata) {
	int index = (int) cbdata;
	struct input data = inputs[index];
	val /= (MAX_UINT16 / JOYSTICK_RANGE) + 1;
	if (val != JOYSTICK_CENTER) {
		printf("%s%d\n", data.tag, JOYSTICK_CENTER - val);
	}
	timeout_add(&t, JOYSTICK_DELAY, read_input, (void *) ((index + 1) % NUM_ELEMENTS(inputs)));
}

void read_input(void *cbdata) {
	int index = (int) cbdata;
	struct input data = inputs[index];
	adc_sample_start(data.channel, read_cb, cbdata);
}

void key_cb(void *cbdata) {
	if (gpio_read(key_pin)) {
		printf("k1\n");
	} else {
		printf("k0\n");
	}
}

volatile int usb_ready = 0;

static void new_data(uint8_t *data, size_t len) {
	cdc_read_more(&cdc);
}

void init_vcdc(int config) {
	cdc_init(new_data, NULL, &cdc);
	cdc_set_stdout(&cdc);
	usb_ready = 1;
	timeout_add(&t, JOYSTICK_DELAY, read_input, 0);
}

void main(void) {
	timeout_init();
	adc_init();
	adc_sample_prepare(ADC_MODE_SINGLE);
	usb_init(&cdc_device);
	while (!usb_ready)
		;

	gpio_dir(key_pin, GPIO_INPUT);
	pin_mode(key_pin, PIN_MODE_PULLUP);
	pin_set_cb(key_pin, PCR_IRQC_INT_EITHER, 1, key_cb, NULL);

	sys_yield_for_frogs();
}
