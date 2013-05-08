#include <sys/types.h>

void
memset(void *addr, int val, size_t len)
{
	char *buf = addr;

	for (; len > 0; --len, ++buf)
		*buf = val;
}

void
memcpy(void *dst, void *src, size_t len)
{
	char *dstbuf = dst;
	char *srcbuf = src;

	for (; len > 0; --len, ++dstbuf, ++srcbuf)
		*dstbuf = *srcbuf;
}
