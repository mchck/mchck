enum onboard_led_state {
        ONBOARD_LED_OFF = 0,
        ONBOARD_LED_ON = 1,
        ONBOARD_LED_TOGGLE = -1,
        ONBOARD_LED_FLOAT = -2
};

void onboard_led(enum onboard_led_state);
