#include <mchck.h>

void
onboard_led(enum onboard_led_state state)
{
        gpio_mode(GPIO_PTC0, GPIO_MODE_OUTPUT | GPIO_MODE_DRIVE_HIGH);

        if (state == ONBOARD_LED_OFF || state == ONBOARD_LED_ON)
                gpio_write(GPIO_PTC0, state);
        else
                gpio_toggle(GPIO_PTC0);
}
