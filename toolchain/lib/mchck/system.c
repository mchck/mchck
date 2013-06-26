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
