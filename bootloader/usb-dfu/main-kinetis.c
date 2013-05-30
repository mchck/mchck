#include <mchck.h>

#include "mcg.h"
#include "sim.h"

#include "usb-kinetis.h"
#include "dfu.h"

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
}

static enum dfu_status
finish_write(size_t off, size_t len)
{
}

void
main(void)
{
//        MCG.dmx32 = 1;           /* mcg at 24MHz */
//        SIM.clkdiv2.usbfrac = 1; /* usb = 2x core */
        OSC_CR = OSC_CR_SC16P_MASK;
        MCG.range0 = MCG_RANGE_VERYHIGH;
        MCG.erefs0 = MCG_EREF_OSC;
        /* MCG.frdiv = 4; */
        /* MCG.clks = MCG_CLKS_EXTERNAL; */
        /* MCG.irefs = 0; */
        MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);

        while (!MCG.oscinit0)
                /* NOTHING */;
        while (MCG.clkst != MCG_CLKST_EXTERNAL)
                /* NOTHING */;

        MCG.prdiv0 = 7;
        MCG.pllclken0 = 1;
        MCG.c6 = 0;
        MCG.vdiv0 = 0;
        MCG.plls = 1;

        while (!MCG.pllst)
                /* NOTHING */;
        while (!MCG.lock0)
                /* NOTHING */;

        MCG.clks = MCG_CLKS_FLLPLL;

        while (MCG.clkst != MCG_CLKST_PLL)
                /* NOTHING */;

        SIM.sopt2.pllfllsel = SIM_PLLFLLSEL_PLL;
        SIM.sopt2.usbsrc = 1;    /* usb from mcg */

        dfu_start(setup_write, finish_write);

        for (;;) {
                usb_intr();
        }
}
