/**
 * Freescale K20 ISR table and startup code.
 */

#include <stdint.h>
#include <mchck-cdefs.h>

#ifndef STACK_SIZE
#define STACK_SIZE 0x400
#endif

__attribute__ ((__section__(".co_stack")))
__attribute__ ((__used__))
static uint32_t sys_stack[STACK_SIZE / 4];

/**
 * What follows is some macro magic to populate the
 * ISR vector table and to declare weak symbols for the handlers.
 *
 * We start by defining the macros VH and V, which by themselves
 * just call the (yet undefined) macro V_handler().
 *
 * V_handler will then be defined separately for each use of the
 * vector list.
 *
 * V_reserved is just used to properly skip the reserved entries
 * in the vector table.
 */

typedef void (isr_handler_t)(void);

isr_handler_t Default_Handler __attribute__((__weak__, __alias__("__Default_Handler")));
isr_handler_t Default_Reset_Handler;


#define VH(num, handler, default)                               \
	V_handler(num, handler, _CONCAT(handler, _Handler), default)
#define V(num, x)                               \
	VH(num, x, Default_Handler)

/**
 * Declare the weak symbols.  By default they will be aliased
 * to Default_Handler, but the default handler can be specified
 * by using VH() instead of V().
 */

#define V_handler(num, name, handler, default)                          \
	isr_handler_t handler __attribute__((__weak__, __alias__(#default)));	\
	isr_handler_t _CONCAT(name, _IRQHandler) __attribute__((__weak__, __alias__(_STR(handler))));
#include "vecs_k20.h"
#undef V_handler

/**
 * Define the vector table.  We simply fill in all (weak) vector symbols
 * and the occasional `0' for the reserved entries.
 */

__attribute__ ((__section__(".isr_vector"), __used__))
isr_handler_t * const isr_vectors[] =
{
	(isr_handler_t *)&sys_stack[sizeof(sys_stack)/sizeof(*sys_stack)],
#define V_handler(num, name, handler, default)	[num] = handler,
#include "vecs_k20.h"
#undef V_handler
};

#undef V
#undef VH


static void
__Default_Handler(void)
{
	for (;;)
		/* NOTHING */;
}

/**
 * The following variables are only used for their addresses;
 * their symbols are defined by the linker script.
 *
 * They are used to delimit various sections the process image:
 * _sidata marks where the flash copy of the .data section starts.
 * _sdata and _edata delimit the RAM addresses of the .data section.
 * _sbss and _ebss delimit the RAM BSS section in the same way.
 */

#include <mchck.h>

void main(void);

void
Default_Reset_Handler(void)
{
	/* Disable Watchdog */
	WDOG_UNLOCK = 0xc520;
	WDOG_UNLOCK = 0xd928;
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;

#ifdef EXTERNAL_XTAL
        OSC_CR = OSC_CR_SC16P_MASK;
        MCG.c2.raw = ((struct MCG_C2_t){
                        .range0 = MCG_RANGE_VERYHIGH,
                                .erefs0 = MCG_EREF_OSC
                                }).raw;
        MCG.c1.raw = ((struct MCG_C1_t){
                        .clks = MCG_CLKS_EXTERNAL,
                                .frdiv = 4, /* log2(EXTERNAL_XTAL) - 20 */
                                .irefs = 0
                                }).raw;

        while (!MCG.s.oscinit0)
                /* NOTHING */;
        while (MCG.s.clkst != MCG_CLKST_EXTERNAL)
                /* NOTHING */;

        MCG.c5.raw = ((struct MCG_C5_t){
                        .prdiv0 = ((EXTERNAL_XTAL / 2000000L) - 1),
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
#else
        /* FLL at 48MHz */
        MCG.c4.raw = ((struct MCG_C4_t){
                        .drst_drs = MCG_DRST_DRS_MID,
                        .dmx32 = 1
                }).raw;
        SIM.sopt2.pllfllsel = SIM_PLLFLLSEL_FLL;
#endif

        memcpy(&_sdata, &_sidata, (uintptr_t)&_edata - (uintptr_t)&_sdata);
        memset(&_sbss, 0, (uintptr_t)&_ebss - (uintptr_t)&_sbss);

        main();
}
