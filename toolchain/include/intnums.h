#include <mchck.h>

#define V(n, x)	_CONCAT(IRQ_, x) = (n - 16),
#define VH(n, x, y)	V(n, x)
enum {
#include "vecs_k20.h"
};
#undef VH
#undef V
