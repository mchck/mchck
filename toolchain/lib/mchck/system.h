void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);

void sys_reset(void);

void crit_enter(void);
void crit_exit(void);

void int_enable(size_t intno);
void int_disable(size_t intno);
