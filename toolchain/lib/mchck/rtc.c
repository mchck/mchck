#include <mchck.h>

void
rtc_init(void)
{
        SIM.scgc6.rtc = 1;
        RTC.cr.osce = 1;
}

int
rtc_start_counter(void)
{
        if (RTC.sr.tif)
                return 1;
        RTC.sr.tce = 1;
        return 0;
}

uint32_t
rtc_get_time()
{
        return RTC.tsr;
}

void
rtc_set_time(uint32_t seconds)
{
        int started = RTC.sr.tce;
        RTC.sr.tce = 0;
        RTC.tsr = seconds;
        RTC.sr.tce = started;
}
