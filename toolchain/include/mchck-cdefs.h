#ifndef _MCHCK_CDEFS_H
#define _MCHCK_CDEFS_H

#include <sys/param.h>

#define _CONCAT(x,y) _CONCAT1(x,y)
#define _CONCAT1(x,y) x ## y
#define _STR(a) #a

//#include <uchar.h>
typedef __CHAR16_TYPE__ char16_t;

#define __packed __attribute__((__packed__))

/* From FreeBSD: compile-time asserts */
#define CTASSERT(x)             _Static_assert(x, _STR(x))

#define CTASSERT_SIZE_BYTE(t, s)     CTASSERT(sizeof(t) == (s))
#define CTASSERT_SIZE_BIT(t, s)     CTASSERT(sizeof(t) * 8 == (s))

#define UNION_STRUCT_START(size)                                \
        union {                                                 \
        _CONCAT(_CONCAT(uint, size), _t) raw;                 \
        struct {                                                \
        /* just to swallow the following semicolon */           \
        struct _CONCAT(_CONCAT(__dummy_, __COUNTER__), _t) {}

#define UNION_STRUCT_END                        \
        }; /* struct */                         \
        }; /* union */

#endif
