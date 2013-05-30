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
        MCG.c2.raw = ((struct MCG_C2_t){
                        .range0 = MCG_RANGE_VERYHIGH,
                                .erefs0 = MCG_EREF_OSC
                                }).raw;
        MCG.c1.raw = ((struct MCG_C1_t){
                        .clks = MCG_CLKS_EXTERNAL,
                                .frdiv = 4,
                                .irefs = 0
                                }).raw;

        while (!MCG.s.oscinit0)
                /* NOTHING */;
        while (MCG.s.clkst != MCG_CLKST_EXTERNAL)
                /* NOTHING */;

        MCG.c5.raw = ((struct MCG_C5_t){
                        .prdiv0 = 7,
                                .pllclken0 = 1
                                }).raw;
        MCG.c6.raw = ((struct MCG_C6_t){
                        .vdiv0 = 0,
                        .plls = 1
                                }).raw;

        while (!MCG.s.pllst)
                /* NOTHING */;
        while (!MCG.s.lock0)
                /* NOTHING */;

        MCG.c1.clks = MCG_CLKS_FLLPLL;

        while (MCG.s.clkst != MCG_CLKST_PLL)
                /* NOTHING */;

        SIM.sopt2.pllfllsel = SIM_PLLFLLSEL_PLL;
        SIM.sopt2.usbsrc = 1;    /* usb from mcg */

        dfu_start(setup_write, finish_write);

        for (;;) {
                usb_intr();
        }
}
