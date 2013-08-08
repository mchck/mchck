#include <mchck.h>

void
gpio_mode(enum gpio_pin_id pin, enum gpio_pin_mode mode)
{
        int pinnum = gpio_physpin_from_pin(pin);
        volatile struct GPIO_t *gpio = gpio_physgpio_from_pin(pin);

        /* enable port clock */
        SIM.scgc5.raw |= 1 << (gpio_portnum_from_pin(pin) + 8);

        /* configure GPIO mode */
        switch (mode) {
        case GPIO_MODE_OUTPUT:
                gpio->pddr |= 1 << pinnum;
                break;
        default:
        case GPIO_MODE_INPUT:
                gpio->pddr &= ~(1 << pinnum);
                break;
        }

        volatile struct PCR_t *pcr = &gpio_physport_from_pin(pin)->pcr[pinnum];
        pcr->mux = PCR_MUX_GPIO;
        pcr->dse = 1;           /* XXX allow strength selection */
}

void
gpio_write(enum gpio_pin_id pin, enum gpio_pin_value val)
{
        BITBAND_BIT(gpio_physgpio_from_pin(pin)->pdor, gpio_physpin_from_pin(pin)) = val;
}

void
gpio_toggle(enum gpio_pin_id pin)
{
        gpio_physgpio_from_pin(pin)->ptor = 1 << gpio_physpin_from_pin(pin);
}

enum gpio_pin_value
gpio_read(enum gpio_pin_id pin)
{
        return (BITBAND_BIT(gpio_physgpio_from_pin(pin)->pdir, gpio_physpin_from_pin(pin)));
}
