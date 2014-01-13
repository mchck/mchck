#include <mchck.h>

// Silence warnings about unused variables
#define UNUSED __attribute__((unused))

struct {
	struct i2c_transaction *cur;

        size_t index;
        enum i2c_state {
                I2C_STATE_IDLE, I2C_STATE_TX, I2C_STATE_RX, I2C_STATE_RX_START
        } state;
} i2c_ctx;

// set up the registers for a new transaction
void
i2c_start_transaction() {
	i2c_ctx.index = 0;
	if(i2c_ctx.cur->read) {		// if this is a read transaction
		i2c_ctx.state = I2C_STATE_RX_START;
		I2C0.c1.raw = ((struct I2C_C1 ) { .iicen = 1, .iicie = 1, .mst = 1, .tx = 1,
						  .txak = i2c_ctx.cur->length == 1 } ).raw;	// transmit acknowledge already when only one byte to receive
		I2C0.d = (i2c_ctx.cur->address << 1) | 1;
	} else {
		i2c_ctx.state = I2C_STATE_TX;
		I2C0.c1.raw = ((struct I2C_C1 ) { .iicen = 1, .iicie = 1, .mst = 1, .tx = 1 } ).raw;
		I2C0.d = i2c_ctx.cur->address << 1;
	}
}

// called when the current transaction is done
void
i2c_transaction_done(enum i2c_result result) {
	// send STOP condition if requested
	if (i2c_ctx.cur->stop)
		I2C0.c1.mst = 0;
	else
		I2C0.c1.rsta = 1;

	// invoke callback if specified
	if (i2c_ctx.cur->cb != NULL)
		(*i2c_ctx.cur->cb)(result, i2c_ctx.cur);

	// if last transaction, go idle, otherwise start next transaction
	i2c_ctx.cur = i2c_ctx.cur->next;	// this is NULL if no next transaction
	if(i2c_ctx.cur == NULL)
		i2c_ctx.state = I2C_STATE_IDLE;
	else
		i2c_start_transaction();
}

void
I2C0_Handler(void)
{
        I2C0.s.iicif = 1;
	onboard_led(ONBOARD_LED_ON);
        enum i2c_result result = I2C_RESULT_SUCCESS;
	switch (i2c_ctx.state) {
		case I2C_STATE_IDLE:
			break;
		case I2C_STATE_TX:
			if (i2c_ctx.index < i2c_ctx.cur->length) {
				if (I2C0.s.rxak) {	// if not acked, consider the transaction done
					result = I2C_RESULT_NACK;
					i2c_transaction_done(result);
				} else {
					I2C0.d = i2c_ctx.cur->buffer[i2c_ctx.index++];
				}
			} else {
				if (I2C0.s.rxak)	// if not acked, report it
					result = I2C_RESULT_NACK;
				i2c_transaction_done(result);
			}
			break;
		case I2C_STATE_RX_START:
			if (I2C0.s.rxak) {		// report premature ack
				result = I2C_RESULT_NACK;
				i2c_transaction_done(result);
			} else {
				I2C0.c1.tx = 0;
				i2c_ctx.state = I2C_STATE_RX;
				// throw away the first byte read from the device.
				UNUSED volatile uint8_t dummy = I2C0.d;
			}
			break;
		case I2C_STATE_RX:
			if (i2c_ctx.index == i2c_ctx.cur->length - 1) {
				if (i2c_ctx.cur->stop)
					I2C0.c1.mst = 0;
				else
					I2C0.c1.rsta = 1;
			} else if (i2c_ctx.index == i2c_ctx.cur->length - 2) {
				I2C0.c1.txak = 1;
			}
			i2c_ctx.cur->buffer[i2c_ctx.index++] = I2C0.d;
			if (i2c_ctx.index == i2c_ctx.cur->length && i2c_ctx.cur->cb != NULL) {
				(*i2c_ctx.cur->cb)(result, i2c_ctx.cur);
				i2c_ctx.cur = i2c_ctx.cur->next;

				if(i2c_ctx.cur == NULL)
					i2c_ctx.state = I2C_STATE_IDLE;
				else
					i2c_start_transaction();
			}
			break;
	}
}

void
i2c_init(enum i2c_rate rate)
{
        // Enable clocks for I2C and PORTB
        SIM.scgc4.i2c0 = 1;
        SIM.scgc5.portb = 1;

        // Enable OPEN_DRAIN and ALT2 on SCK and SDA
        // Note enabling PULLUP (.ps=1,.pe=1) has no effect in I2C mode.
        PORTB.pcr[pin_physpin_from_pin(PIN_PTB2)].raw = ((struct PCR_t) {.mux=2,.ode=1}).raw;
        PORTB.pcr[pin_physpin_from_pin(PIN_PTB3)].raw = ((struct PCR_t) {.mux=2,.ode=1}).raw;

        //                   I2C0_F values, indexed by enum i2c_rate.
        //                   100kHz 400   600   800   1000  1200  1500  2000  2400kHz
        static uint8_t f[] = {0x27, 0x85, 0x14, 0x45, 0x0D, 0x0B, 0x09, 0x02, 0x00};

        if (rate < 0 || rate >= sizeof(f))
            rate = I2C_RATE_100;
        I2C0.f.raw = f[rate];

        // Filter glitches on the I2C bus, filter glitches up to 4 bus cycles long.
        //I2C0.flt = 4;

        I2C0.c1.iicen = 1;
        i2c_ctx.state = I2C_STATE_IDLE;
	i2c_ctx.cur = NULL;
        int_enable(IRQ_I2C0);
}

void
i2c_queue(struct i2c_transaction *transaction) {
	transaction->next = NULL;		// make sure the new transaction has no leftover 'next' value, since the library handles queueing

	if(i2c_ctx.cur == NULL) {		// if no current transaction, start this one right away
		i2c_ctx.cur = transaction;
		i2c_start_transaction();
	} else {				// otherwise, go to end of queue and add transaction there
		struct i2c_transaction *it = i2c_ctx.cur;
		while(it->next != NULL)
			it = it->next;
		it->next = transaction;
	}
}
