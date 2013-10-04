#include <mchck.h>

int
main(void)
{
	SIM.scgc6.ftm0 = 1;
	FTM0.mod = 0xffff;
	FTM0.cntin = 0;
	FTM0.channel[0].csc.msb = 1;
	FTM0.channel[0].csc.elsb = 1;
	FTM0.channel[0].cv = 0x8000;
	FTM0.sc.clks = 1;

	for (;;)
		/* NOTHING */;
}
