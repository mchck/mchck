enum pit_id {
	PIT_0 = 0x0,
	PIT_1 = 0x1,
	PIT_2 = 0x2,
	PIT_3 = 0x3
};

typedef void (pit_callback)(enum pit_id id);

void pit_init(void);
void pit_start(enum pit_id id, uint32_t cycles, pit_callback *cb);
void pit_stop(enum pit_id id);
uint32_t pit_cycle(enum pit_id id);
