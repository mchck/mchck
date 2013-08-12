enum gpio_port_id {
        GPIO_PORTA = 1 << 16,
        GPIO_PORTB = 2 << 16,
        GPIO_PORTC = 3 << 16,
        GPIO_PORTD = 4 << 16,
};

enum gpio_pin_id {
        GPIO_PTA0 = GPIO_PORTA | 0,
        GPIO_PTA1 = GPIO_PORTA | 1,
        GPIO_PTA2 = GPIO_PORTA | 2,
        GPIO_PTA3 = GPIO_PORTA | 3,
        GPIO_PTA4 = GPIO_PORTA | 4,
        GPIO_PTA19 = GPIO_PORTA | 19,
        GPIO_PTB0 = GPIO_PORTB | 0,
        GPIO_PTB1 = GPIO_PORTB | 1,
        GPIO_PTB2 = GPIO_PORTB | 2,
        GPIO_PTB3 = GPIO_PORTB | 3,
        GPIO_PTB16 = GPIO_PORTB | 16,
        GPIO_PTB17 = GPIO_PORTB | 17,
        GPIO_PTC0 = GPIO_PORTC | 0,
        GPIO_PTC1 = GPIO_PORTC | 1,
        GPIO_PTC2 = GPIO_PORTC | 2,
        GPIO_PTC3 = GPIO_PORTC | 3,
        GPIO_PTC4 = GPIO_PORTC | 4,
        GPIO_PTC5 = GPIO_PORTC | 5,
        GPIO_PTC6 = GPIO_PORTC | 6,
        GPIO_PTC7 = GPIO_PORTC | 7,
        GPIO_PTD0 = GPIO_PORTD | 0,
        GPIO_PTD1 = GPIO_PORTD | 1,
        GPIO_PTD2 = GPIO_PORTD | 2,
        GPIO_PTD3 = GPIO_PORTD | 3,
        GPIO_PTD4 = GPIO_PORTD | 4,
        GPIO_PTD5 = GPIO_PORTD | 5,
        GPIO_PTD6 = GPIO_PORTD | 6,
        GPIO_PTD7 = GPIO_PORTD | 7,
};

enum gpio_pin_mode {
        GPIO_MODE_UNKNOWN = 0,
        GPIO_MODE_OUTPUT,
        GPIO_MODE_INPUT
};

enum gpio_pin_value {
        GPIO_LOW = 0,
        GPIO_HIGH = 1
};

static inline enum gpio_port_id
gpio_port_from_pin(enum gpio_pin_id pin)
{
        return (pin & 0xf0000);
}

static inline int
gpio_portnum_from_pin(enum gpio_pin_id pin)
{
        return (gpio_port_from_pin(pin) >> 16);
}

static inline volatile struct PORT_t *
gpio_physport_from_pin(enum gpio_pin_id pin)
{
        switch (gpio_port_from_pin(pin)) {
        case GPIO_PORTA:
                return (&PORTA);
        case GPIO_PORTB:
                return (&PORTB);
        case GPIO_PORTC:
                return (&PORTC);
        case GPIO_PORTD:
                return (&PORTD);
        default:
                return (NULL);
        }
}

static inline volatile struct GPIO_t *
gpio_physgpio_from_pin(enum gpio_pin_id pin)
{
        switch (gpio_port_from_pin(pin)) {
        case GPIO_PORTA:
                return (&GPIOA);
        case GPIO_PORTB:
                return (&GPIOB);
        case GPIO_PORTC:
                return (&GPIOC);
        case GPIO_PORTD:
                return (&GPIOD);
        default:
                return (NULL);
        }
}

static inline int
gpio_physpin_from_pin(enum gpio_pin_id pin)
{
        return (pin & 0xffff);
}

void gpio_mode(enum gpio_pin_id pin, enum gpio_pin_mode mode);
void gpio_write(enum gpio_pin_id pin, enum gpio_pin_value value);
void gpio_toggle(enum gpio_pin_id pin);
enum gpio_pin_value gpio_read(enum gpio_pin_id);
