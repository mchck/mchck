#include <mchck.h>

extern struct {
        uint32_t stack;
        void (*reset)(void) __attribute__((__noreturn__));
} _app;

void
Reset_Handler(void)
{
	__set_MSP(_app.stack);
	_app.reset();
}
