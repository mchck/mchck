#include <mchck.h>

#include "wire.h"

#define UNUSED __attribute__((unused))

#define BUFFER_LENGTH 32

struct wire_s {
    uint8_t rxBuffer[BUFFER_LENGTH];
    uint8_t rxBufferIndex;
    uint8_t rxBufferLength;

    uint8_t txAddress;
    uint8_t txBuffer[BUFFER_LENGTH];
    uint8_t txBufferIndex;
    uint8_t txBufferLength;

    uint8_t transmitting;
};

struct wire_s wire;

static void i2c_wait(void) {
	while (!(I2C0.s.iicif))
		;
	I2C0.s.iicif = 1;
}

void wire_begin() {
	// Enable I2C clock
	SIM.scgc4.i2c0 = 1;

	// On Teensy 3.0 external pullup resistors *MUST* be used
	// the PORT_PCR_PE bit is ignored when in I2C mode
	// I2C will not work at all without pullup resistors
	pin_mode(PIN_PTB2, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);
	pin_mode(PIN_PTB3, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);

	I2C0.f = (struct I2C_F ) { .mult = I2C_MULT_1, .icr = 0x1B };
	I2C0.c2.hdrs = 1;
	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
}

void wire_beginTransmission(uint8_t address) {
	wire.txBuffer[0] = (address << 1);
	wire.transmitting = 1;
	wire.txBufferLength = 1;
}

uint8_t wire_endTransmission(uint8_t sendStop) {
	uint8_t i;
	struct I2C_S status;

	// clear the status flags
	I2C0.s.iicif = 1;
	I2C0.s.arbl = 1;
	// now take control of the bus...
	if (I2C0.c1.mst) {
		// we are already the bus master, so send a repeated start
		//		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1, .rsta=1} ).raw;
	} else {
		// we are not currently the bus master, so wait for bus ready
		while (I2C0.s.busy)
			;
		// become the bus master in transmit mode (send start)
		//		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;
	}
	// transmit the address and data
	for (i=0; i < wire.txBufferLength; i++) {
		I2C0.d = wire.txBuffer[i];
		i2c_wait();
		status = I2C0.s;
		if (status.rxak) {
			// the slave device did not acknowledge
			break;
		}
		if ((status.arbl)) {
			// we lost bus arbitration to another master
			// TODO: what is the proper thing to do here??
			break;
		}
	}
	if (sendStop) {
		// send the stop condition
		//		I2C0_C1 = I2C_C1_IICEN;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
		// TODO: do we wait for this somehow?
	}
	wire.transmitting = 0;
	return 0;
}

uint8_t wire_requestFrom(uint8_t address, uint8_t length, uint8_t sendStop) {
	UNUSED uint8_t tmp;
	uint8_t count=0;
	struct I2C_S status;

	wire.rxBufferIndex = 0;
	wire.rxBufferLength = 0;

	// clear the status flags
	I2C0.s.iicif = 1;
	I2C0.s.arbl = 1;
	// now take control of the bus...
	if (I2C0.c1.mst) {
		// we are already the bus master, so send a repeated start
		//		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_RSTA | I2C_C1_TX;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1, .rsta=1} ).raw;
	} else {
		// we are not currently the bus master, so wait for bus ready
		while (I2C0.s.busy)
			;
		// become the bus master in transmit mode (send start)
		//		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;
	}
	// send the address
	I2C0.d = (address << 1) | 1;
	i2c_wait();
	status = I2C0.s;
	if ((status.rxak) || (status.arbl)) {
		// the slave device did not acknowledge
		// or we lost bus arbitration to another master
		//		I2C0_C1 = I2C_C1_IICEN;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
		return 0;
	}
	if (length == 0) {
		// TODO: does anybody really do zero length reads?
		// if so, does this code really work?
		//		I2C0_C1 = I2C_C1_IICEN | (sendStop ? 0 : I2C_C1_MST);
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst = sendStop ? 0 : 1} ).raw;
		return 0;
	} else if (length == 1) {
		//		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TXAK;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .txak=1} ).raw;
	} else {
		//		I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1} ).raw;
	}
	tmp = I2C0.d; // initiate the first receive
	while (length > 1) {
		i2c_wait();
		length--;
		if (length == 1) {
			//			I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TXAK;
			I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .txak=1} ).raw;
		}
		wire.rxBuffer[count++] = I2C0.d;
	}
	i2c_wait();
	//	I2C0_C1 = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;
	I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .mst=1, .tx=1} ).raw;

	wire.rxBuffer[count++] = I2C0.d;
	if (sendStop) {
		//		I2C0_C1 = I2C_C1_IICEN;
		I2C0.c1.raw = ((struct I2C_C1) {.iicen=1} ).raw;
	}
	wire.rxBufferLength = count;
	return count;
}

size_t wire_write(uint8_t data) {
	if (wire.transmitting) {
		if (wire.txBufferLength >= BUFFER_LENGTH+1) {
			return 0;
		}
		wire.txBuffer[wire.txBufferLength++] = data;
	} else {
		// TODO: implement slave mode
	}
	return 0;
}

int wire_read(void) {
	if (wire.rxBufferIndex >= wire.rxBufferLength) return -1;
	return wire.rxBuffer[wire.rxBufferIndex++];
}

int wire_available(void) {
	return wire.rxBufferLength - wire.rxBufferIndex;
}
