#include <mchck.h>

#define PWM_CHNUM 8

enum pwm_channels {
    PWM_CH0 = 0x1,
    PWM_CH1 = 0x2,
    PWM_CH2 = 0x4,
    PWM_CH3 = 0x8,
    PWM_CH4 = 0x10,
    PWM_CH5 = 0x20,
    PWM_CH6 = 0x40,
    PWM_CH7 = 0x80,

    PWM_ALL = 0xFF
} ;

void pwm_init(enum pwm_channels channels) {
    SIM.scgc6.ftm0 = 1;             /* enable clock to ftm0  */
    FTM0.mode.ftmen = 1;            /* enable new FTM functionality */
    FTM0.sc.ps  = FTM_PS_DIV1;      /* don't prescale clock */
    FTM0.mod = 48e3;                /* should give a 1kHz pwm output */
    FTM0.sc.clks = FTM_CLKS_SYSTEM; /* system clock */

    for(int i = 0; i < PWM_CHNUM; i++) {
        if(channels & (1<<i)) {
            FTM0.csc[i].msb = 1;            /* edge-aligned PWM on channel i */
            FTM0.csc[i].elsb = 1;           /* active-high output */
        }
    }
}

void pwm_set(enum pwm_channels channels, uint16_t width) {
    for(int i = 0; i < PWM_CHNUM; i++)
        if(channels & (1<<i))
            FTM0.cv[i].val = width;
}

int
main(void)
{
    pin_mode(PIN_PTC1, PIN_MODE_MUX_ALT4);

    pwm_init(PWM_CH0);
    pwm_set(PWM_CH0, 1e3);

	for (;;)
        /* NOTHING */;
}
