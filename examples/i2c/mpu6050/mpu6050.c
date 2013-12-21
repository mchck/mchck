#include <mchck.h>
#include "mpu6050.desc.h"

#define MPU6050_ADDR    0x68
#define TIMEOUT_REPEAT  1000

static struct cdc_ctx cdc;
static struct timeout_ctx t;

void
part1(void *cbdata);

void
part3(enum i2c_status status, uint8_t *data, size_t length, void *cbdata)
{
        printf("whoami: 0x%02x\r\n", data[0]);
        timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);
}

void
part2(enum i2c_status status, uint8_t *data, size_t length, void *cbdata)
{
        static uint8_t buffer[1];
        i2c_recv(MPU6050_ADDR, buffer, sizeof(buffer), I2C_STOP, part3, NULL);
}

void
part1(void *cbdata)
{
        static uint8_t cmd[] = { 117 };
        i2c_send(MPU6050_ADDR, cmd, sizeof(cmd), I2C_NOSTOP, part2, NULL);
}

static void
new_data(uint8_t *data, size_t len)
{
        cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
        cdc_init(new_data, NULL, &cdc);
        cdc_set_stdout(&cdc);
        timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);
}

void
main(void)
{
        timeout_init();
        i2c_init(I2C_RATE_100);
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
