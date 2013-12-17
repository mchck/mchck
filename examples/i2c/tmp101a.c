#include <mchck.h>
#include "tmp101a.desc.h"

static struct cdc_ctx cdc;

static void new_data(uint8_t *data, size_t len) {
    cdc_read_more(&cdc);
}

static volatile int usb_ready = 0;

void init_vcdc(int config) {
    cdc_init(new_data, NULL, &cdc);
    cdc_set_stdout(&cdc);
    usb_ready = 1;
}

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

#define TMP101_ADDR             0x48
#define TEMPERATURE_REGISTER    0x00
#define CONFIGURATION_REGISTER  0x01
#define CONFIG_12_BITS          0b01100000

#define TIMEOUT_REPEAT          1000

void part1(void *cbdata);

void part4(uint8_t *data, size_t length, void *cbdata) {
    // Work in 1000ths of a degree to allow rounding to 100ths
    long c = data[0] * 1000 + (int) ((data[1] * 1000) / 256);
    long f = c * 9 / 5 + 32000;
    printf("temperature: %ld.%02ldC %ld.%02ldF\r\n",
            c / 1000, ((c + 5) % 1000) / 10, f / 1000, ((f + 5) % 1000) / 10);
    timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);
}

void part3(uint8_t *sent, size_t length, void *cbdata) {
    static uint8_t buffer[2];
    i2c_recv(TMP101_ADDR, buffer, sizeof(buffer), I2C_STOP, part4, NULL);
}

void part2(uint8_t *sent, size_t length, void *cbdata) {
    static uint8_t cmd[] = { TEMPERATURE_REGISTER };
    i2c_send(TMP101_ADDR, cmd, sizeof(cmd), I2C_NOSTOP, part3, NULL);
}

void part1(void *cbdata) {
    blink(10);
    static uint8_t cmd[] = { CONFIGURATION_REGISTER, CONFIG_12_BITS };
    i2c_send(TMP101_ADDR, cmd, sizeof(cmd), I2C_NOSTOP, part2, NULL);
}

void main(void) {
    usb_init(&cdc_device);
    while  (!usb_ready)
        ;
    timeout_init();
    i2c_init(I2C_RATE_100);

    timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);

    sys_yield_for_frogs();
}
