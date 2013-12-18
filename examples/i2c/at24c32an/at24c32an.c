#include <mchck.h>
#include "at24c32an.desc.h"

static struct cdc_ctx cdc;
static struct timeout_ctx t;

void delay(int n) {
    while (n--)
        asm("nop");
}

void blink(int n) {
    onboard_led(ONBOARD_LED_ON);
    delay(n);
    onboard_led(ONBOARD_LED_OFF);
    delay(n);
}

#define DEVICE_ADDR             0x50

#define TIMEOUT_REPEAT          1000

void part1(void *cbdata);

void part3(uint8_t *data, size_t length, void *cbdata) {
    timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);
}

void part2(uint8_t *sent, size_t length, void *cbdata) {
    static uint8_t buffer[1];
    i2c_recv(DEVICE_ADDR, buffer, sizeof(buffer), I2C_STOP, part3, NULL);
}

void part1(void *cbdata) {
    blink(10);
    static uint8_t cmd[] = { 0, 0 };
    i2c_send(DEVICE_ADDR, cmd, sizeof(cmd), I2C_NOSTOP, part2, NULL);
}

static void new_data(uint8_t *data, size_t len) {
    cdc_read_more(&cdc);
}

void init_vcdc(int config) {
    cdc_init(new_data, NULL, &cdc);
    cdc_set_stdout(&cdc);
    timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);
}

void main(void) {
    timeout_init();
    i2c_init(I2C_RATE_100);
    usb_init(&cdc_device);
    sys_yield_for_frogs();
}
