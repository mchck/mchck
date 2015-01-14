#include <mchck.h>

void
ftm_init(void)
{
        SIM.scgc6.ftm0 = 1;     // turn on clock to ftm0
	FTM0.mod = 0xffff;      // set counter modulo value to maximum
	FTM0.cntin = 0;

        for (int i = 0; i < FTM_NUM_CH; i++) {
                FTM0.channel[i].csc.msb = 1;
                FTM0.channel[i].csc.elsb = 1;
                FTM0.channel[i].cv = 0x8000;
        }

        FTM0.sc.clks = FTM_CLKS_SYSTEM;       // select clock for counter
}

void
ftm_set_raw(enum FTM_CH_t channels, uint16_t duty)
{
        for (int i = 0; i <= FTM_NUM_CH; i++) {
                if(channels & (1 << i))
                        FTM0.channel[i].cv = duty;
        }
}
