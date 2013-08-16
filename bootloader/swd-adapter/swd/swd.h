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
#else
  #include <mchck.h>

  enum swd_pin {
          SWD_DIO_PIN = GPIO_PTB16,
          SWD_CLK_PIN = GPIO_PTB3
  };

  enum swd_pin_mode {
          SWD_MODE_OUTPUT = GPIO_MODE_OUTPUT | GPIO_MODE_SLEW_SLOW,
          SWD_MODE_INPUT = GPIO_MODE_INPUT | GPIO_MODE_FILTER_ON
  };
#endif

#ifdef __cplusplus
extern "C" {
#endif

const uint8_t *process_buf(const uint8_t *buf, size_t len);
void pin_configure(enum swd_pin pin, enum swd_pin_mode mode);
void pin_write(enum swd_pin pin, int val);
int pin_read(enum swd_pin pin);
size_t reply_space(void);
void reply_write(const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /*SWD_H__*/
