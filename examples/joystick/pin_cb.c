#include <mchck.h>

struct pin_cb_t {
	void (*cb)(void *cbdata);
	void *cbdata;
} pin_cbs[32];

void PORTD_Handler(void) {
	for (int i = 0; i < 32; i++) {
		struct pin_cb_t *pin_cb = &pin_cbs[i];

		if (pin_cb->cb) {
			if (PORTD.pcr[i].isf) {
				PORTD.pcr[i].isf = 1; /* clear isf */
				(*pin_cb->cb)(pin_cb->cbdata);
			}
		} else {
			PORTD.pcr[i].isf = 1; /* clear isf */
		}
	}
//	PORTD.isfr = 0xFFFFFFFF;
}

void pin_clear_cb(enum pin_id pin)
{
	int pinnum = pin_physpin_from_pin(pin);

	pin_cbs[pinnum].cb = 0;
	volatile struct PORT_t *port = pin_physport_from_pin(pin);
	port->pcr[pinnum].irqc = PCR_IRQC_DISABLED;
}

void pin_set_cb(enum pin_id pin, enum PCR_IRQC_t irqc, int filter, void (*cb)(void *cbdata), void *cbdata) {
	int pinnum = pin_physpin_from_pin(pin);

	pin_cbs[pinnum].cb = cb;
	pin_cbs[pinnum].cbdata = cbdata;

	volatile struct PORT_t *port = pin_physport_from_pin(pin);

	if (filter) {
		port->dfcr.cs = PORT_CS_LPO;
		port->dfwr.filt = 31;
		port->dfer |= 1 << pinnum;
	} else {
		port->dfer &= ~(1 << pinnum);
	}

	port->pcr[pinnum].irqc = irqc;

	enum pin_port_id port_id = pin_port_from_pin(pin);
	switch (port_id) {
	case PIN_PORTA:
		int_enable(IRQ_PORTA);
		break;
	case PIN_PORTB:
		int_enable(IRQ_PORTB);
		break;
	case PIN_PORTC:
		int_enable(IRQ_PORTC);
		break;
	case PIN_PORTD:
		int_enable(IRQ_PORTD);
		break;
	}
}
