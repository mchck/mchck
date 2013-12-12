#include <mchck.h>
#include "tmp101a.desc.h"

static struct cdc_ctx cdc;

static void new_data(uint8_t *data, size_t len) {
    cdc_read_more(&cdc);
}

void init_vcdc(int config) {
    cdc_init(new_data, NULL, &cdc);
    cdc_set_stdout(&cdc);
}

static struct timeout_ctx t;

void delay(int n) {
    while(n--)
        asm("nop");
}

void blink(int n) {
    onboard_led(ONBOARD_LED_ON);
    delay(n);
    onboard_led(ONBOARD_LED_OFF);
    delay(n);
}

#define TMP101_ADDR 0x48
#define TEMPERATURE_REGISTER    0x00
#define CONFIGURATION_REGISTER  0x01
#define CONFIG_12_BITS          0b01100000

#define TIMEOUT_REPEAT          1000

void part1(void *cbdata);

void part4(uint8_t *data, int length, void *cbdata) {
    blink(50);
    // Work in 1000ths of a degree to allow rounding to 100ths
    long c = data[0] * 1000 + (int)((data[1]*1000)/256);
    long f = c * 9 / 5 + 32000;
    printf("temperature: %ld.%02ldC %ld.%02ldF\r\n",
        c / 1000, ((c + 5) % 1000) / 10,
        f / 1000, ((f + 5) % 1000) / 10);
	timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);
}

void part3(uint8_t *sent, int length, void *cbdata) {
    static uint8_t buffer[2];
    i2c_recv(TMP101_ADDR, buffer, sizeof(buffer), part4, NULL);
}

void part2(uint8_t *sent, int length, void *cbdata) {
    static uint8_t cmd[] = { TMP101_ADDR, TEMPERATURE_REGISTER };
    i2c_send(cmd, sizeof(cmd), part3, NULL);
}

void part1(void *cbdata) {
    static uint8_t cmd[] = { TMP101_ADDR, CONFIGURATION_REGISTER, CONFIG_12_BITS };
    i2c_send(cmd, sizeof(cmd), part2, NULL);
}

void main(void) {
    timeout_init();
    usb_init(&cdc_device);
    i2c_init();

	timeout_add(&t, TIMEOUT_REPEAT, part1, NULL);

	sys_yield_for_frogs();
}
