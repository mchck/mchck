void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *addr, int val, size_t len);
size_t strlen(const char *str);
char *strchr(const char *str, int chr);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t len);


void __attribute__((noreturn)) sys_reset(void);
void __attribute__((noreturn)) sys_reset_to_loader(void);
void __attribute__((noreturn)) sys_yield_for_frogs(void);

void crit_enter(void);
void crit_exit(void);
int crit_active(void);

void __attribute__((noreturn)) panic(const char *reason);

void int_enable(size_t intno);
void int_disable(size_t intno);

static inline volatile uint32_t *
bitband_bitp(volatile void *addr, size_t bit)
{
        return ((volatile void *)(0x42000000 + ((uintptr_t)addr - 0x40000000) * 32 + 4 * bit));
}

#define BITBAND_BIT(var, bit)	(*bitband_bitp(&(var), (bit)))

#define KKASSERT(cond)                          \
        if (!(cond)) {                          \
                panic(_STR(cond));              \
        }

extern const uint8_t sys_reset_to_loader_magic[22];
