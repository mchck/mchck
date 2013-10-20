#ifndef SWD_H__
#define SWD_H__

#ifdef ARDUINO
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

enum swd_pin {
SWD_DIO_PIN = 11,
	SWD_CLK_PIN = 12
};

enum swd_pin_mode {
	SWD_MODE_OUTPUT = OUTPUT,
	SWD_MODE_INPUT = INPUT
};
#elif MSP430
#include <msp430.h>
#include <legacymsp430.h>
#include <stdlib.h>
#include <signal.h>

enum swd_pin {
	SWD_DIO_PIN = BIT5,
	SWD_CLK_PIN = BIT4
};

enum swd_pin_mode {
	SWD_MODE_OUTPUT,
	SWD_MODE_INPUT
};
#else
#include <mchck.h>

enum swd_pin {
	SWD_DIO_PIN = GPIO_PTA3,
	SWD_CLK_PIN = GPIO_PTA0
};

enum swd_pin_mode {
	SWD_MODE_OUTPUT = GPIO_OUTPUT,
	SWD_MODE_INPUT = GPIO_INPUT
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

int process_data(const uint8_t *buf, size_t len);
const uint8_t *process_command(const uint8_t *buf, size_t len, int *outpipe_full);
void signal_led(void);
void pin_configure(enum swd_pin pin, enum swd_pin_mode mode);
void pin_write(enum swd_pin pin, int val);
int pin_read(enum swd_pin pin);
int outpipe_space(size_t len);
void reply_write(const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /*SWD_H__*/
