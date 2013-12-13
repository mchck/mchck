enum i2c_stop {I2C_NOSTOP, I2C_STOP};
typedef void (i2c_cb)(uint8_t *buffer, int length, void *cbdata);
void i2c_init();
void i2c_recv(uint8_t address, uint8_t *data, int length, enum i2c_stop stop, i2c_cb *cb, void *cbdata);
void i2c_send(uint8_t *data, int length, enum i2c_stop stop, i2c_cb *cb, void *cbdata);
