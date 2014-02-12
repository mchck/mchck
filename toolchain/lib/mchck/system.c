#include <mchck.h>

const uint8_t sys_reset_to_loader_magic[] = "\xff\x00\x7fRESET TO LOADER\x7f\x00\xff";


void
sys_reset(void)
{
        SCB.aircr.raw = ((struct SCB_AIRCR_t){
                        .vectkey = SCB_AIRCR_VECTKEY,
                                .sysresetreq = 1
                                }).raw;
        for (;;);
}

void
sys_reset_to_loader(void)
{
        memcpy(&VBAT, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic));
        sys_reset();
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

int
crit_active(void)
{
        return (crit_nest != 0);
}

static volatile const char *panic_reason;

void __attribute__((noreturn))
panic(const char *reason)
{
        crit_enter();
        panic_reason = reason;

        for (;;)
                /* infinite loop */;
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
