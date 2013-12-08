void pin_set_cb(enum pin_id pin, enum PCR_IRQC_t irqc, int filter, void (*cb)(void *cbdata), void *cbdata);
void pin_clear_cb(enum pin_id pin);
