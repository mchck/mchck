#include <mchck.h>

volatile struct GPIO_t *
gpio_physgpio_from_pin(enum gpio_pin_id pin)
{
        switch (pin_port_from_pin(pin)) {
        case PIN_PORTA:
                return (&GPIOA);
        case PIN_PORTB:
                return (&GPIOB);
        case PIN_PORTC:
                return (&GPIOC);
        case PIN_PORTD:
                return (&GPIOD);
        default:
                return (NULL);
        }
}

void
gpio_dir(enum gpio_pin_id pin, enum gpio_dir dir)
{
        int pinnum = pin_physpin_from_pin(pin);
        volatile struct GPIO_t *pinp = gpio_physgpio_from_pin(pin);

        switch (dir) {
        case GPIO_OUTPUT:
                pinp->pddr |= 1 << pinnum;
                goto set_mux;
        case GPIO_INPUT:
                pinp->pddr &= ~(1 << pinnum);
        set_mux:
                pin_mode(pin, PIN_MODE_MUX_ALT1);
                break;
        case GPIO_DISABLE:
                pin_mode(pin, PIN_MODE_MUX_ANALOG);
                break;
        }
}

void
gpio_write(enum gpio_pin_id pin, enum gpio_pin_value val)
{
        BITBAND_BIT(gpio_physgpio_from_pin(pin)->pdor, pin_physpin_from_pin(pin)) = val;
}

void
gpio_toggle(enum gpio_pin_id pin)
{
        gpio_physgpio_from_pin(pin)->ptor = 1 << pin_physpin_from_pin(pin);
}

enum gpio_pin_value
gpio_read(enum gpio_pin_id pin)
{
        return (BITBAND_BIT(gpio_physgpio_from_pin(pin)->pdir, pin_physpin_from_pin(pin)));
}
