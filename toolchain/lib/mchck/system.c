#include <mchck.h>

void
sys_reset(void)
{
        SCB.aircr.raw = ((struct SCB_AIRCR_t){
                        .vectkey = SCB_AIRCR_VECTKEY,
                                .sysresetreq = 1
                                }).raw;
        for (;;);
}

static int crit_nest;

void
crit_enter(void)
{
        __asm__("cpsid i");
        crit_nest++;
}

void
crit_exit(void)
{
        if (--crit_nest == 0)
                __asm__("cpsie i");
}
