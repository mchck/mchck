/**
 * On Kinetis MCUs, the GPIO and pin (PORT) numbering is identical.
 */
enum gpio_pin_id {
        GPIO_PTA0 = PIN_PTA0,
        GPIO_PTA1 = PIN_PTA1,
        GPIO_PTA2 = PIN_PTA2,
        GPIO_PTA3 = PIN_PTA3,
        GPIO_PTA4 = PIN_PTA4,
        GPIO_PTA18 = PIN_PTA18,
        GPIO_PTA19 = PIN_PTA19,
        GPIO_PTB0 = PIN_PTB0,
        GPIO_PTB1 = PIN_PTB1,
        GPIO_PTB2 = PIN_PTB2,
        GPIO_PTB3 = PIN_PTB3,
        GPIO_PTB16 = PIN_PTB16,
        GPIO_PTB17 = PIN_PTB17,
        GPIO_PTC0 = PIN_PTC0,
        GPIO_PTC1 = PIN_PTC1,
        GPIO_PTC2 = PIN_PTC2,
        GPIO_PTC3 = PIN_PTC3,
        GPIO_PTC4 = PIN_PTC4,
        GPIO_PTC5 = PIN_PTC5,
        GPIO_PTC6 = PIN_PTC6,
        GPIO_PTC7 = PIN_PTC7,
        GPIO_PTD0 = PIN_PTD0,
        GPIO_PTD1 = PIN_PTD1,
        GPIO_PTD2 = PIN_PTD2,
        GPIO_PTD3 = PIN_PTD3,
        GPIO_PTD4 = PIN_PTD4,
        GPIO_PTD5 = PIN_PTD5,
        GPIO_PTD6 = PIN_PTD6,
        GPIO_PTD7 = PIN_PTD7,
};

enum gpio_dir {
        GPIO_INPUT,
        GPIO_OUTPUT,
        GPIO_DISABLE
};

enum gpio_pin_value {
        GPIO_LOW = 0,
        GPIO_HIGH = 1
};

void gpio_dir(enum gpio_pin_id pin, enum gpio_dir dir);
void gpio_write(enum gpio_pin_id pin, enum gpio_pin_value value);
void gpio_toggle(enum gpio_pin_id pin);
enum gpio_pin_value gpio_read(enum gpio_pin_id);
