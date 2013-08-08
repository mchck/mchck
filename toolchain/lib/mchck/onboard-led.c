

#include <mchck.h>

void
onboard_led(enum onboard_led_state state)
{
        gpio_mode(GPIO_PC0, GPIO_MODE_OUTPUT);

        if (state == ONBOARD_LED_OFF || state == ONBOARD_LED_ON)
                gpio_write(GPIO_PC0, state);
        else
                gpio_toggle(GPIO_PC0);
}
