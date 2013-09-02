void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *addr, int val, size_t len);
size_t strlen(const char *str);

void sys_reset(void);
void __attribute__((noreturn)) sys_yield_for_frogs(void);

void crit_enter(void);
void crit_exit(void);

void int_enable(size_t intno);
void int_disable(size_t intno);

static inline volatile uint32_t *
bitband_bitp(volatile void *addr, size_t bit)
{
        return ((volatile void *)(0x42000000 + ((uintptr_t)addr - 0x40000000) * 32 + 4 * bit));
}

#define BITBAND_BIT(var, bit)	(*bitband_bitp(&(var), (bit)))
