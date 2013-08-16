#include <stdlib.h>

void
sys_reset(void)
{
        exit(0);
}

void
crit_enter(void)
{
        /* dummy for now, maybe later block signals. */
}

void
crit_exit(void)
{
        /* ditto */
}
