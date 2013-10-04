#include <mchck.h>

int
main(void)
{
    int inc = 1;
    long fract v = 0.0;
    ftm_init();
	pin_mode(PIN_PTC1, PIN_MODE_MUX_ALT4);

	for (;;) {
        if(inc)
            v += 0.00001lr;
        else
            v -= 0.00001lr;

        if(v >= 1 || v <= 0) inc = !inc;

        ftm_set(FTM_CH0, v);
    }
}
