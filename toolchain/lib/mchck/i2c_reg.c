#include <mchck.h>

static void
i2c_reg_read_3(enum i2c_result result, struct i2c_transaction *trans)
{
        struct i2c_reg_read_ctx *ctx = (struct i2c_reg_read_ctx *) trans;
        ctx->cb(trans->buffer, result, trans->cbdata);
}

static void
i2c_reg_read_2(enum i2c_result result, struct i2c_transaction *trans)
{
        struct i2c_reg_read_ctx *ctx = (struct i2c_reg_read_ctx *) trans;
        trans->direction = I2C_READ;
        trans->stop = I2C_STOP;
        trans->length = ctx->len;
        trans->cb = i2c_reg_read_3;
        i2c_queue(trans);
}

void
i2c_reg_read(struct i2c_reg_read_ctx *ctx, uint8_t address, uint8_t reg,
             uint8_t *buf, uint8_t len, i2c_reg_read_cb *cb, void *cbdata)
{
        buf[0] = reg;
        ctx->len = len;
        ctx->cb = cb;
        ctx->trans.address = address;
        ctx->trans.direction = I2C_WRITE;
        ctx->trans.stop = I2C_NOSTOP;
        ctx->trans.buffer = buf;
        ctx->trans.length = 1;
        ctx->trans.cb = i2c_reg_read_2;
        ctx->trans.cbdata = cbdata;
        i2c_queue(&ctx->trans);
}
