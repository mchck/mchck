#include <mchck.h>

static struct pit_ctx_t {
	pit_callback *cb;
} ctx[4];

void
pit_init(void)
{
	SIM.scgc6.pit = 1;
	PIT.mcr.mdis = 0;
	PIT.mcr.frz = 0;
	int_enable(IRQ_PIT0);
	int_enable(IRQ_PIT1);
	int_enable(IRQ_PIT2);
	int_enable(IRQ_PIT3);
}

void
pit_start(enum pit_id id, uint32_t cycles, pit_callback *cb)
{
	ctx[id].cb = cb;
	volatile struct PIT_TIMER_t *timer = &PIT.timer[id];
	timer->ldval = cycles;
	timer->tflg.tif = 1;
	timer->tctrl.tie = cb != NULL;
	timer->tctrl.ten = 1;
}

void
pit_stop(enum pit_id id)
{
	PIT.timer[id].tctrl.ten = 0;
}

uint32_t
pit_cycle(enum pit_id id)
{
	return PIT.timer[id].cval;
}

static void
common_handler(enum pit_id id)
{
	PIT.timer[id].tflg.tif = 1;
	ctx[id].cb(id);
}

void
PIT0_Handler(void)
{
	common_handler(PIT_0);
}

void
PIT1_Handler(void)
{
	common_handler(PIT_1);
}

void
PIT2_Handler(void)
{
	common_handler(PIT_2);
}

void
PIT3_Handler(void)
{
	common_handler(PIT_3);
}
