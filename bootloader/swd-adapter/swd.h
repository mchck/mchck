#include <mchck.h>

enum swd_pin {
        SWD_DIO_PIN = GPIO_PTB16,
        SWD_CLK_PIN = GPIO_PTB3
};

enum swd_pin_mode {
        SWD_MODE_OUTPUT = GPIO_MODE_OUTPUT,
        SWD_MODE_INPUT = GPIO_MODE_INPUT
};

const uint8_t *process_buf(const uint8_t *buf, size_t len);
void pin_configure(enum swd_pin pin, enum swd_pin_mode mode);
void pin_write(enum swd_pin pin, int val);
int pin_read(enum swd_pin pin);
size_t reply_space(void);
void reply_write(const uint8_t *buf, size_t len);
