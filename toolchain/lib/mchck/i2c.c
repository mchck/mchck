#include <mchck.h>

// Silence warnings about unused variables
#define UNUSED __attribute__((unused))

static i2c_cb *rxCb;
static void *rxCbdata;
static uint8_t *rxBuffer;
static int rxLength;
static int rxIndex;

static i2c_cb *txCb;
static void *txCbdata;
static int txLength;
static int txIndex;
static uint8_t *txBuffer;

static enum i2c_state {
    I2C_STATE_IDLE,
    I2C_STATE_TX,
    I2C_STATE_RX,
    I2C_STATE_RX_START
} state;

void I2C0_Handler(void) {
    I2C0.s.iicif = 1;
    switch (state) {
    case I2C_STATE_IDLE:
        break;
    case I2C_STATE_TX:
        if (txIndex < txLength) {
            if (I2C0.s.rxak) {
                I2C0.c1.mst = 0;
                state = I2C_STATE_IDLE;
                if (txCb) {
                    (*txCb)(txBuffer, txIndex, txCbdata);
                }
            } else {
                I2C0.d = txBuffer[txIndex++];
            }
        } else {
            I2C0.c1.mst = 0;
            state = I2C_STATE_IDLE;
            if (txCb) {
                (*txCb)(txBuffer, txIndex, txCbdata);
            }
        }
        break;
    case I2C_STATE_RX_START:
        if (I2C0.s.rxak) {
            I2C0.c1.mst = 0;
            state = I2C_STATE_IDLE;
            if (rxCb) {
                (*rxCb)(rxBuffer, rxIndex, rxCbdata);
            }
        } else {
            I2C0.c1.tx = 0;
            state = I2C_STATE_RX;
            UNUSED volatile uint8_t dummy = I2C0.d;
        }
        break;
    case I2C_STATE_RX:
        if (rxIndex == rxLength - 1) {
            I2C0.c1.mst = 0;
            state = I2C_STATE_IDLE;
        } else if (rxIndex == rxLength - 2) {
            I2C0.c1.txak = 1;
        }
        rxBuffer[rxIndex++] = I2C0.d;
        if (rxIndex == rxLength && rxCb) {
            (*rxCb)(rxBuffer, rxLength, rxCbdata);
        }
        break;
    }
}

void i2c_init()
{
	SIM.scgc5.portb = 1;
	pin_mode(PIN_PTB2, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);
	pin_mode(PIN_PTB3, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);

    SIM.scgc4.i2c0 = 1;
    I2C0.f = (struct I2C_F ) { .mult = I2C_MULT_1, .icr = 0x1B };
    I2C0.c1.iicen = 1;
    state = I2C_STATE_IDLE;
    int_enable(IRQ_I2C0);
}

void i2c_recv(uint8_t address, uint8_t *data, int length, i2c_cb *cb, void *cbdata) {
    blink(20);
    
    rxBuffer = data;
    rxLength = length;
    rxIndex = 0;
    rxCb = cb;
    rxCbdata = cbdata;
    state = I2C_STATE_RX_START;
    I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .mst=1, .tx=1} ).raw;
    I2C0.d = (address << 1) | 1;
}

void i2c_send(uint8_t *data, int length, i2c_cb *cb, void *cbdata) {
    blink(20);

    txBuffer = data;
    txLength = length;
    txIndex = 0;
    txCb = cb;
    txCbdata = cbdata;
    state = I2C_STATE_TX;
    I2C0.c1.raw = ((struct I2C_C1) {.iicen=1, .iicie=1, .mst=1, .tx=1} ).raw;
    I2C0.d = txBuffer[txIndex++] << 1;
}
