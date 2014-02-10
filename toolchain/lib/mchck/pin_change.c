#include <mchck.h>

void
pin_change_handler(volatile struct PORT_t *port,
                   const struct pin_change_handler *handlers,
                   const struct pin_change_handler *end)
{
        const struct pin_change_handler *i;
        for (i = handlers; i < end; i++) {
                uint32_t bit = 1 << pin_physpin_from_pin(i->pin_id);
                if (port->isfr & bit) {
                        port->isfr = bit;
                        i->cb(i->cbdata);
                }
        }
}

#define PORT_CHANGE_HANDLER(port) \
        extern const struct pin_change_handler pin_hooks_##port, pin_hooks_##port##_end; \
        \
        void \
        PORT##port##_Handler(void) \
        { \
                pin_change_handler(&PORT##port, &pin_hooks_##port, \
                                   &pin_hooks_##port##_end);       \
        }

PORT_CHANGE_HANDLER(A);
PORT_CHANGE_HANDLER(B);
PORT_CHANGE_HANDLER(C);
PORT_CHANGE_HANDLER(D);

#define pin_change_init_port(port, scgc_shift)                          \
        for (const struct pin_change_handler *i = &pin_hooks_##port;    \
             i < &pin_hooks_##port##_end; i++) {                        \
                SIM.scgc5.raw |= 1 << scgc_shift;                       \
                volatile struct PORT_t *p = &PORT##port;                \
                p->pcr[pin_physpin_from_pin(i->pin_id)].irqc = i->polarity; \
                int_enable(IRQ_PORT##port);                             \
        }

void
pin_change_init(void)
{
        pin_change_init_port(A, 9);
        pin_change_init_port(B, 10);
        pin_change_init_port(C, 11);
        pin_change_init_port(D, 12);
}
