#include <mchck.h>

void
onboard_led(enum onboard_led_state state)
{
        /* Enable PORTC clock */
        SIM.scgc5.portc = 1;

        /* Configure pin as GPIO */
        PORTC.pcr[0].mux = PCR_MUX_GPIO;
        PORTC.pcr[0].dse = 1;

        /* Configure pin as output */
        GPIOC.pddr |= 1 << 0;

        switch (state) {
        case ONBOARD_LED_OFF:
                GPIOC.pcor = 1 << 0;
                break;
        case ONBOARD_LED_ON:
                GPIOC.psor = 1 << 0;
                break;
        default:
                GPIOC.ptor = 1 << 0;
        }
}
