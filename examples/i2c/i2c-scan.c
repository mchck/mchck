// tries to address every possible I2C address (128 total) to produce a little 16x8 map over USB-serial.

#include <mchck.h>
#include "i2c-scan.desc.h"

#define TIMEOUT_REPEAT  5000

static struct cdc_ctx cdc;
static struct timeout_ctx t;

// we keep a single transaction struct around to reuse
static struct i2c_transaction probe;

void
initiate_send(void *cbdata);

void
send_complete(enum i2c_result result, struct i2c_transaction *transaction)
{
	// print column header
	if (probe.address == 0)
		printf("\r\n  0123456789ABCDEF\r\n");

	// print row header
	if (probe.address % 16 == 0)
		printf("%x:", probe.address / 16);

	// print result
	printf("%c", (result == I2C_RESULT_SUCCESS) ? 'X' : '.');

	probe.address++;
	probe.address &= 0x7f;			// Constrain the address to be between 0 and 128, 7 bits

	// print end of line
	if (probe.address % 16 == 0)
		printf("\r\n");

	// scan next address
	uint32_t delay = probe.address == 0 ? TIMEOUT_REPEAT : 0;
	timeout_add(&t, delay, initiate_send, NULL);
}

void
initiate_send(void *cbdata)
{
	i2c_queue(&probe);
}

static void
new_data(uint8_t *data, size_t len)
{
	cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
	cdc_init(new_data, NULL, &cdc);
	cdc_set_stdout(&cdc);

	// set up I2C transaction struct
	probe.address = 0;
	probe.direction = I2C_WRITE;
	probe.buffer = NULL;
	probe.length = 0;
	probe.cb = send_complete;
	probe.cbdata = NULL;

	timeout_add(&t, 10, initiate_send, NULL);
}

void
main(void)
{
	timeout_init();
	i2c_init(I2C_RATE_100);
	usb_init(&cdc_device);
	sys_yield_for_frogs();
}
