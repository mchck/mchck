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

void __attribute__((noreturn))
sys_yield_for_frogs(void)
{
        SCB.scr.sleeponexit = 1;
        for (;;)
                __asm__("wfi");
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

void
int_enable(size_t intno)
{
        NVIC.iser[intno / 32] = 1 << (intno % 32);
}

void
int_disable(size_t intno)
{
        NVIC.icer[intno / 32] = 1 << (intno % 32);
}
