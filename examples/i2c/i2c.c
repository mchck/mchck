#include <mchck.h>
#include <stdfix.h>

#include "i2c.desc.h"

#include "t3.h"
#include "wire.h"

// Silence warnings about unused variables
#define UNUSED __attribute__((unused))

// Silence warnings about %k,%K,%r,%R in printfs
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

#define TMP101_ADDR	0x48
#define	TEMPERATURE_REGISTER	0x00
#define	CONFIGURATION_REGISTER	0x01
#define	CONFIG_12_BITS			0b01100000

static struct cdc_ctx cdc;

static void new_data(uint8_t *data, size_t len) {
	// ignore data
	cdc_read_more(&cdc);
}

void init_vcdc(int config) {
	cdc_init(new_data, NULL, &cdc);
	cdc_set_stdout(&cdc);
}

static struct timeout_ctx t;

//#define SMEDING
#ifdef SMEDING
#define MPU6050_ADDR 0b1101000

struct acc_value {
	int16_t x_accel;
	int16_t y_accel;
	int16_t z_accel;
	int16_t temperature;
	int16_t x_gyro;
	int16_t y_gyro;
	int16_t z_gyro;
};

struct i2c_ctx {
	enum i2c_state {
		I2C_STATE_IDLE, I2C_STATE_RX, I2C_STATE_TX
	} state;

	uint8_t *cur_buf;
	int cur_buf_idx;
	unsigned int cur_buf_len;
} ctx;

void i2c_init() {
	// turn on clock to I2C module
	SIM.scgc4.i2c0 = 1;

	// set I2C rate
	I2C0.f = (struct I2C_F ) { .mult = I2C_MULT_1, .icr = 0x1B };   // XXX: check values, possibly determine dynamically

	// enable I2C and interrupt
	I2C0.c1.iicen = 1;
	I2C0.c1.iicie = 1;
	int_enable(IRQ_I2C0);

	// initialize state
	ctx.state = I2C_STATE_IDLE;
	ctx.cur_buf = NULL;
	ctx.cur_buf_idx = 0;
	ctx.cur_buf_len = 0;
}

static const enum gpio_pin_id led_pin = GPIO_PTC0;

// pretty straight implementation of page 1036 of the K20 Sub-Family Reference Manual
void I2C0_Handler(void) {
	gpio_write(led_pin, GPIO_HIGH);
	I2C0.s.iicif = 1;   // clear interrupt flag
	switch (ctx.state) {
	case I2C_STATE_IDLE:
		// we shouldn't get an interrupt when idle
		break;

	case I2C_STATE_RX:
		if (ctx.cur_buf_idx == -1) { // if we haven't received a byte yet, the previous operation was the address transmit
			if (I2C0.s.rxak) {       // if no ack,
				I2C0.c1.mst = 0;    // generate STOP
			} else {
				// start the actual receiving
				I2C0.c1.tx = 0;
				ctx.cur_buf_idx = 0;
				UNUSED volatile uint8_t dummy = I2C0.d;    // dummy read
			}
		} else {
			if (ctx.cur_buf_idx == ctx.cur_buf_len - 1) {      // if last byte,
				I2C0.c1.mst = 0;                // generate STOP
				ctx.state = I2C_STATE_IDLE;
			} else if (ctx.cur_buf_idx == ctx.cur_buf_len - 2) {   // if last-but-one byte,
				I2C0.c1.txak = 1;               // send acknowledge
			}

			ctx.cur_buf[ctx.cur_buf_idx++] = I2C0.d;        // store received data
		}
		break;

	case I2C_STATE_TX:
		if (ctx.cur_buf_idx == ctx.cur_buf_len - 1) {  // if last byte,
			ctx.state = I2C_STATE_IDLE;
			I2C0.c1.mst = 0;            // generate STOP
		} else {
			if (I2C0.s.rxak)             // if no ack,
				I2C0.c1.mst = 0;        // generate STOP
			else
				I2C0.d = ctx.cur_buf[ctx.cur_buf_idx++];

		}
		break;
	}
}

void i2c_send(uint8_t address, uint8_t *data, unsigned int len) {
	// change ctx
	ctx.state = I2C_STATE_TX;
	ctx.cur_buf = data;
	ctx.cur_buf_idx = 0;
	ctx.cur_buf_len = len;

	// kick off transmission by sending address
	I2C0.c1.tx = 1;
	I2C0.c1.mst = 1;
	I2C0.d = (address << 1) | 0x00; // LSB = 0 => write transaction
}

void i2c_receive(uint8_t address, uint8_t *data, unsigned int len) {
	// change ctx
	ctx.state = I2C_STATE_RX;
	ctx.cur_buf = data;
	ctx.cur_buf_idx = -1;   // first byte will be address
	ctx.cur_buf_len = len;

	// kick off reception by sending address
	I2C0.c1.tx = 1;
	I2C0.c1.mst = 1;
	I2C0.d = (address << 1) | 0x01; // LSB = 1 => read transaction
}

void read_mpu6050(void *cbdata) {
	// receive whoami data
	uint8_t whoami_req[1] = { 0x75 };
	uint8_t whoami = 0;

	i2c_send(MPU6050_ADDR, whoami_req, 1);
	while (ctx.state != I2C_STATE_IDLE)
		;
	i2c_receive(MPU6050_ADDR, &whoami, 1);
	while (ctx.state != I2C_STATE_IDLE)
		;
	printf("whoami: %x\r\n", whoami);

	timeout_add(&t, 1000, read_mpu6050, NULL);
}

void read_tmp101(void *cbdata) {
	onboard_led(ONBOARD_LED_ON);

	uint8_t configure[] = { CONFIGURATION_REGISTER, CONFIG_12_BITS };
	i2c_send(TMP101_ADDR, configure, sizeof(configure));
	onboard_led(ONBOARD_LED_OFF);
	while (ctx.state != I2C_STATE_IDLE)
		;
	uint8_t read_temp[] = { TEMPERATURE_REGISTER };
	i2c_send(TMP101_ADDR, read_temp, sizeof(configure));
	while (ctx.state != I2C_STATE_IDLE)
		;
	uint8_t temperature[2];
	i2c_receive(TMP101_ADDR, temperature, sizeof(temperature));
	while (ctx.state != I2C_STATE_IDLE)
		;

	printf("temperature: %x %x\r\n", temperature[0], temperature[1]);
}
#endif

void wire_read_tmp101(void *cbdata) {
	onboard_led(ONBOARD_LED_ON);

	wire_begin();

	wire_beginTransmission(TMP101_ADDR);
	wire_write(CONFIGURATION_REGISTER);
	wire_write(CONFIG_12_BITS);
	wire_endTransmission(0);

	wire_beginTransmission(TMP101_ADDR);
	wire_write(TEMPERATURE_REGISTER);
	wire_endTransmission(0);

	uint8_t data[2];
	wire_requestFrom(TMP101_ADDR, sizeof(data), 1);
	data[0] = wire_read();
	data[1] = wire_read();
	accum c = data[0] + data[1] / 256k;
	accum f = c * 9k / 5k + 32k;
	printf("temperature: %.2k\r\n", f);

	onboard_led(ONBOARD_LED_OFF);

	timeout_add(&t, 1000, wire_read_tmp101, NULL);
}

void t3_read_tmp101(void *cbdata) {
	onboard_led(ONBOARD_LED_ON);

	t3_begin(I2C_MASTER, 0, 0, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_100);

	t3_beginTransmission(TMP101_ADDR);
	t3_write(CONFIGURATION_REGISTER);
	t3_write(CONFIG_12_BITS);
	onboard_led(ONBOARD_LED_OFF);
	t3_endTransmission(I2C_STOP);

	t3_beginTransmission(TMP101_ADDR);
	t3_write(TEMPERATURE_REGISTER);
	t3_endTransmission(I2C_NOSTOP);

	uint8_t data[2];
	t3_requestFrom(TMP101_ADDR, sizeof(data), I2C_STOP);
	data[0] = t3_read();
	data[1] = t3_read();
	accum c = data[0] + data[1] / 256k;
	accum f = c * 9k / 5k + 32k;
	printf("temperature: %.2k\r\n", f);


//	timeout_add(&t, 1000, t3_read_tmp101, NULL);
}

void main(void) {
	SIM.scgc5.portb = 1;

	pin_mode(PIN_PTB2, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);
	pin_mode(PIN_PTB3, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);

	onboard_led(ONBOARD_LED_OFF);

	timeout_init();
	usb_init(&cdc_device);
#ifdef SMEDING
	i2c_init();
#endif

	timeout_add(&t, 1000, t3_read_tmp101, NULL);
	sys_yield_for_frogs();
}
