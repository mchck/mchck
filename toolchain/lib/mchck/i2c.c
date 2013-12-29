#include <mchck.h>

// Silence warnings about unused variables
#define UNUSED __attribute__((unused))

struct i2c_s {
        enum i2c_stop sendStop;
        i2c_cb *cb;
        void *cbdata;
        uint8_t *buffer;
        size_t length;
        size_t index;
        enum i2c_state {
                I2C_STATE_IDLE, I2C_STATE_TX, I2C_STATE_RX, I2C_STATE_RX_START
        } state;
} i2c;

void
I2C0_Handler(void)
{
        I2C0.s.iicif = 1;
        enum i2c_result result = I2C_RESULT_SUCCESS;
        switch (i2c.state) {
        case I2C_STATE_IDLE:
                break;
        case I2C_STATE_TX:
                if (i2c.index < i2c.length) {
                        if (I2C0.s.rxak) {
                                if (i2c.sendStop)
                                        I2C0.c1.mst = 0;
                                else
                                        I2C0.c1.rsta = 1;
                                i2c.state = I2C_STATE_IDLE;
                                if (i2c.cb) {
                                        (*i2c.cb)(result, i2c.buffer, i2c.index, i2c.cbdata);
                                }
                        } else {
                                I2C0.d = i2c.buffer[i2c.index++];
                        }
                } else {
                        if (!I2C0.s.rxak)
                                result = I2C_RESULT_NACK;
                        if (i2c.sendStop)
                                I2C0.c1.mst = 0;
                        else
                                I2C0.c1.rsta = 1;
                        i2c.state = I2C_STATE_IDLE;
                        if (i2c.cb) {
                                (*i2c.cb)(result, i2c.buffer, i2c.index, i2c.cbdata);
                        }
                }
                break;
        case I2C_STATE_RX_START:
                if (I2C0.s.rxak) {
                        if (i2c.sendStop)
                                I2C0.c1.mst = 0;
                        else
                                I2C0.c1.rsta = 1;
                        i2c.state = I2C_STATE_IDLE;
                        if (i2c.cb) {
                                (*i2c.cb)(result, i2c.buffer, i2c.index, i2c.cbdata);
                        }
                } else {
                        I2C0.c1.tx = 0;
                        i2c.state = I2C_STATE_RX;
                        // Throw away the first byte read from the device.
                        UNUSED volatile uint8_t dummy = I2C0.d;
                }
                break;
        case I2C_STATE_RX:
                if (i2c.index == i2c.length - 1) {
                        if (i2c.sendStop)
                                I2C0.c1.mst = 0;
                        else
                                I2C0.c1.rsta = 1;
                        i2c.state = I2C_STATE_IDLE;
                } else if (i2c.index == i2c.length - 2) {
                        I2C0.c1.txak = 1;
                }
                i2c.buffer[i2c.index++] = I2C0.d;
                if (i2c.index == i2c.length && i2c.cb) {
                        (*i2c.cb)(result, i2c.buffer, i2c.length, i2c.cbdata);
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

        pin_mode(PIN_PTB2, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);
        pin_mode(PIN_PTB3, PIN_MODE_MUX_ALT2 | PIN_MODE_PULLUP | PIN_MODE_OPEN_DRAIN_ON);

        //                   I2C0_F values, indexed by enum i2c_rate.
        //                   100kHz 400   600   800   1000  1200  1500  2000  2400kHz
        static uint8_t f[] = {0x27, 0x85, 0x14, 0x45, 0x0D, 0x0B, 0x09, 0x02, 0x00};

        if (rate < 0 || rate >= sizeof(f))
            rate = I2C_RATE_100;
        I2C0.f.raw = f[rate];

        // Filter glitches on the I2C bus, filter glitches up to 4 bus cycles long.
        //I2C0_FLT = 4;

        I2C0.c1.iicen = 1;
        i2c.state = I2C_STATE_IDLE;
        int_enable(IRQ_I2C0);
}

void
i2c_recv(uint8_t address,
         uint8_t *data,
         size_t length,
         enum i2c_stop stop,
         i2c_cb *cb,
         void *cbdata)
{
        i2c.buffer = data;
        i2c.length = length;
        i2c.index = 0;
        i2c.cb = cb;
        i2c.cbdata = cbdata;
        i2c.sendStop = stop;
        i2c.state = I2C_STATE_RX_START;

        // txak needs to be set on the second to last byte,
        // if we only have one data byte then that is NOW!
        I2C0.c1.raw = ((struct I2C_C1 ) { .iicen = 1, .iicie = 1, .mst = 1, .tx = 1,
                                          .txak = i2c.length == 1 } ).raw;
        I2C0.d = (address << 1) | 1;
}

void
i2c_send(uint8_t address,
         uint8_t *data,
         size_t length,
         enum i2c_stop stop,
         i2c_cb *cb,
         void *cbdata)
{
        i2c.buffer = data;
        i2c.length = length;
        i2c.index = 0;
        i2c.cb = cb;
        i2c.cbdata = cbdata;
        i2c.sendStop = stop;
        i2c.state = I2C_STATE_TX;

        I2C0.c1.raw = ((struct I2C_C1 ) { .iicen = 1, .iicie = 1, .mst = 1, .tx = 1 } ).raw;
        I2C0.d = address << 1;
}
