enum i2c_mode   {I2C_MASTER, I2C_SLAVE};
enum i2c_pins   {I2C_PINS_18_19, I2C_PINS_16_17};
enum i2c_pullup {I2C_PULLUP_EXT, I2C_PULLUP_INT};
enum i2c_rate   {I2C_RATE_100,
                 I2C_RATE_200,
                 I2C_RATE_300,
                 I2C_RATE_400,
                 I2C_RATE_600,
                 I2C_RATE_800,
                 I2C_RATE_1000,
                 I2C_RATE_1200,
                 I2C_RATE_1500,
                 I2C_RATE_2000,
                 I2C_RATE_2400};
enum i2c_stop   {I2C_NOSTOP, I2C_STOP};
enum i2c_status {I2C_WAITING,
                 I2C_SENDING,
                 I2C_SEND_ADDR,
                 I2C_RECEIVING,
                 I2C_TIMEOUT,
                 I2C_ADDR_NAK,
                 I2C_DATA_NAK,
                 I2C_ARB_LOST,
                 I2C_SLAVE_TX,
                 I2C_SLAVE_RX};

void t3_begin(enum i2c_mode mode, uint8_t address1, uint8_t address2, enum i2c_pins pins, enum i2c_pullup pullup, enum i2c_rate rate);
void t3_beginTransmission(uint8_t address);
uint8_t t3_endTransmission(enum i2c_stop sendStop);
void t3_sendTransmission(enum i2c_stop sendStop);                       // non-blocking
size_t t3_requestFrom(uint8_t addr, size_t len, enum i2c_stop sendStop);
void t3_sendRequest(uint8_t addr, size_t len, enum i2c_stop sendStop);  // non-blocking
enum i2c_status t3_status(void);
uint8_t t3_done(void);
uint8_t t3_finish(void);
size_t t3_write(const uint8_t data);
int t3_available(void);
int t3_read(void);
int t3_peek(void);
uint8_t t3_readByte(void); // returns 0 if empty
uint8_t t3_peekByte(void); // returns 0 if empty
void t3_flush(void);
uint8_t t3_getRxAddr(void);
void t3_onReceive(void (*user_onReceive)(size_t len));
void t3_onRequest(void (*user_onRequest)(void));
