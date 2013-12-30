enum i2c_stop {
        I2C_NOSTOP,
        I2C_STOP
};

enum i2c_rate {
        I2C_RATE_100 = 0,
        I2C_RATE_400,
        I2C_RATE_600,
        I2C_RATE_800,
        I2C_RATE_1000,
        I2C_RATE_1200,
        I2C_RATE_1500,
        I2C_RATE_2000,
        I2C_RATE_2400
};

enum i2c_result {
        I2C_RESULT_SUCCESS,
        I2C_RESULT_NACK
};

typedef void (i2c_cb)(enum i2c_result result,
                      uint8_t *buffer,
                      size_t length,
                      void *cbdata);

void
i2c_init(enum i2c_rate rate);

void
i2c_recv(uint8_t address,
         uint8_t *data,
         size_t length,
         enum i2c_stop stop,
         i2c_cb *cb,
         void *cbdata);

void
i2c_send(uint8_t address,
         uint8_t *data,
         size_t length,
         enum i2c_stop stop,
         i2c_cb *cb,
         void *cbdata);
