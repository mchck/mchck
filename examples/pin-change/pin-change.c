#include <mchck.h>

int
main(void)
{
        pin_mode(PIN_PTB1, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO | PIN_MODE_FILTER_ON);
        pin_change_init();
        sys_yield_for_frogs();
}

void
pin_changed(void *cbdata)
{
        onboard_led(ONBOARD_LED_TOGGLE);
}

PIN_DEFINE_CALLBACK(PIN_PTB1, PIN_CHANGE_FALLING, pin_changed, NULL);
