/* reading byte-wide I2C registers
 *
 * Reading register is a common pattern in I2C interfaces. A register
 * read consists of a WRITE transaction to set the register address to
 * be read followed by a READ transaction to fetch the value. This
 * interface abstracts out this pattern.
 */

typedef void (i2c_reg_read_cb)(uint8_t *buf, enum i2c_result result, void *cbdata);

struct i2c_reg_read_ctx {
        struct i2c_transaction trans;
        uint8_t len;
        i2c_reg_read_cb *cb;
};

void i2c_reg_read(struct i2c_reg_read_ctx *ctx, uint8_t address, uint8_t reg,
                  uint8_t *buf, uint8_t len, i2c_reg_read_cb *cb, void *cbdata);
