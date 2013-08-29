#include <mchck.h>

void
gpio_mode(enum gpio_pin_id pin, enum gpio_pin_mode mode)
{
        int pinnum = gpio_physpin_from_pin(pin);

        /* enable port clock */
        SIM.scgc5.raw |= 1 << (gpio_portnum_from_pin(pin) + 8);

        volatile struct GPIO_t *gpio = gpio_physgpio_from_pin(pin);
        struct PCR_t pcr = gpio_physport_from_pin(pin)->pcr[pinnum];

        if (mode & GPIO_MODE_RESET) {
                pcr.raw &= ~0xff;
                pcr.mux = 0;
        }

        /* configure GPIO mode */
        switch (mode & GPIO_MODE__DIRECTION_MASK) {
        case GPIO_MODE_OUTPUT:
                gpio->pddr |= 1 << pinnum;
                goto set_mux;
        case GPIO_MODE_INPUT:
                gpio->pddr &= ~(1 << pinnum);
set_mux:
                pcr.mux = PCR_MUX_GPIO;
                break;
        }

        switch (mode & GPIO_MODE__SLEW_MASK) {
        case GPIO_MODE_SLEW_FAST:
                pcr.sre = 0;
                break;
        case GPIO_MODE_SLEW_SLOW:
                pcr.sre = 1;
                break;
        }

        switch (mode & GPIO_MODE__PULL_MASK) {
        case GPIO_MODE_PULL_OFF:
                pcr.pe = 0;
                break;
        case GPIO_MODE_PULLDOWN:
                pcr.pe = 1;
                pcr.ps = PCR_PULLDOWN;
                break;
        case GPIO_MODE_PULLUP:
                pcr.pe = 1;
                pcr.ps = PCR_PULLUP;
                break;
        }

        switch (mode & GPIO_MODE__DRIVE_MASK) {
        case GPIO_MODE_DRIVE_LOW:
                pcr.dse = 0;
                break;
        case GPIO_MODE_DRIVE_HIGH:
                pcr.dse = 1;
                break;
        }

        switch (mode & GPIO_MODE__FILTER_MASK) {
        case GPIO_MODE_FILTER_OFF:
                pcr.pfe = 0;
                break;
        case GPIO_MODE_FILTER_ON:
                pcr.pfe = 1;
                break;
        }

        switch (mode & GPIO_MODE__OPEN_DRAIN_MASK) {
        case GPIO_MODE_OPEN_DRAIN_OFF:
                pcr.ode = 0;
                break;
        case GPIO_MODE_OPEN_DRAIN_ON:
                pcr.ode = 1;
                break;
        }

        gpio_physport_from_pin(pin)->pcr[pinnum] = pcr;
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
