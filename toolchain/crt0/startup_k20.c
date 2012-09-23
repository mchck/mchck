/**
 * Freescale K20 ISR table and startup code.
 */

#include <stdint.h>

#ifndef STACK_SIZE
#define STACK_SIZE 0x400
#endif

__attribute__ ((__section__(".co_stack")))
__attribute__ ((__used__))
static uint32_t sys_stack[STACK_SIZE / 4 * 4];

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


#define _CONCAT(a, b) a ## b
#define _STR(a) #a
#define VH(handler, default)			\
	V_handler(handler, _CONCAT(handler, _Handler), default)
#define V(x)					\
	VH(x, Default_Handler)

/**
 * Declare the weak symbols.  By default they will be aliased
 * to Default_Handler, but the default handler can be specified
 * by using VH() instead of V().
 */

#define V_handler(n, h, d)						\
	isr_handler_t h __attribute__((__weak__, __alias__(#d)));	\
	isr_handler_t _CONCAT(n, _IRQHandler) __attribute__((__weak__, __alias__(_STR(h))));
#define V_reserved()
#include "vecs_k20.h"
#undef V_handler
#undef V_reserved

/**
 * Define the vector table.  We simply fill in all (weak) vector symbols
 * and the occasional `0' for the reserved entries.
 */

__attribute__ ((__section__(".isr_vector")))
isr_handler_t * const isr_vectors[] =
{
	(isr_handler_t *)&sys_stack[sizeof(sys_stack)/sizeof(*sys_stack)],
#define V_handler(n, h, d)	h,
#define V_reserved()	0,
#include "vecs_k20.h"
#undef V
#undef V_Reserved
};


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

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

void SystemInit(void);
void main(void);

void
Default_Reset_Handler(void)
{
	uint32_t *src, *dst;

	for (dst = &_sdata, src = &_sidata; dst < &_edata; ++src, ++dst)
		*dst = *src;

	for (dst = &_sbss; dst < &_ebss; ++dst)
		*dst = 0;

	SystemInit();

	main();
}
