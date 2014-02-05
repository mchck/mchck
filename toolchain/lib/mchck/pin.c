#include <mchck.h>

volatile struct PORT_t *
pin_physport_from_pin(enum pin_id pin)
{
        switch (pin_port_from_pin(pin)) {
        case PIN_PORTA:
                return (&PORTA);
        case PIN_PORTB:
                return (&PORTB);
        case PIN_PORTC:
                return (&PORTC);
        case PIN_PORTD:
                return (&PORTD);
        default:
                return (NULL);
        }
}

void
pin_mode(enum pin_id pin, enum pin_mode mode)
{
        int pinnum = pin_physpin_from_pin(pin);

        /* enable port clock */
        SIM.scgc5.raw |= 1 << (pin_portnum_from_pin(pin) + 8);

        struct PCR_t pcr = pin_physport_from_pin(pin)->pcr[pinnum];

        if (mode & PIN_MODE_RESET) {
                pcr.raw &= ~0xff;
                pcr.mux = 0;
        }

        if (mode & PIN_MODE__SLEW) {
                switch (mode & PIN_MODE__SLEW_MASK) {
                case PIN_MODE_SLEW_FAST:
                        pcr.sre = 0;
                        break;
                case PIN_MODE_SLEW_SLOW:
                        pcr.sre = 1;
                        break;
                }
        }

        if (mode & PIN_MODE__PULL) {
                switch (mode & PIN_MODE__PULL_MASK) {
                case PIN_MODE_PULL_OFF:
                        pcr.pe = 0;
                        break;
                case PIN_MODE_PULLDOWN:
                        pcr.pe = 1;
                        pcr.ps = PCR_PULLDOWN;
                        break;
                case PIN_MODE_PULLUP:
                        pcr.pe = 1;
                        pcr.ps = PCR_PULLUP;
                        break;
                }
        }

        if (mode & PIN_MODE__DRIVE) {
                switch (mode & PIN_MODE__DRIVE_MASK) {
                case PIN_MODE_DRIVE_LOW:
                        pcr.dse = 0;
                        break;
                case PIN_MODE_DRIVE_HIGH:
                        pcr.dse = 1;
                        break;
                }
        }

        if (mode & PIN_MODE__FILTER) {
                switch (mode & PIN_MODE__FILTER_MASK) {
                case PIN_MODE_FILTER_OFF:
                        pcr.pfe = 0;
                        break;
                case PIN_MODE_FILTER_ON:
                        pcr.pfe = 1;
                        break;
                }
        }

        if (mode & PIN_MODE__OPEN_DRAIN) {
                switch (mode & PIN_MODE__OPEN_DRAIN_MASK) {
                case PIN_MODE_OPEN_DRAIN_OFF:
                        pcr.ode = 0;
                        break;
                case PIN_MODE_OPEN_DRAIN_ON:
                        pcr.ode = 1;
                        break;
                }
        }

        if (mode & PIN_MODE__MUX) {
                pcr.mux = (mode & PIN_MODE__MUX_MASK) >> 13; /* XXX */
        }

        pin_physport_from_pin(pin)->pcr[pinnum] = pcr;
}

void
pin_change_handler(volatile struct PORT_t *port,
                   struct pin_change_handler *handlers,
                   struct pin_change_handler *end)
{
        struct pin_change_handler *i;
        for (i = handlers; i < end; i++) {
                uint32_t bit = 1 << pin_physpin_from_pin(i->pin_id);
                if (port->isfr & bit) {
                        port->isfr = bit;
                        i->cb(i->cbdata);
                }
        }
}

#define PORT_CHANGE_HANDLER(port) \
        extern struct pin_change_handler pin_hooks_##port, pin_hooks_##port##_end; \
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

#define pin_change_init_port(port, scgc_shift)                 \
        for (struct pin_change_handler *i = &pin_hooks_##port; \
             i < &pin_hooks_##port##_end; i++) {               \
                SIM.scgc5.raw |= 1 << scgc_shift;              \
                volatile struct PORT_t *p = &PORT##port;       \
                p->pcr[pin_physpin_from_pin(i->pin_id)].irqc = i->polarity; \
        }

/* Invoked from startup_k20.c */
void
pin_change_init()
{
        pin_change_init_port(A, 9);
        pin_change_init_port(B, 10);
        pin_change_init_port(C, 11);
        pin_change_init_port(D, 12);
}
