#include <mchck.h>

void *
memset(void *addr, int val, size_t len)
{
	char *buf = addr;

	for (; len > 0; --len, ++buf)
		*buf = val;
	return (addr);
}

void *
memcpy(void *dst, const void *src, size_t len)
{
	char *dstbuf = dst;
	const char *srcbuf = src;

	for (; len > 0; --len, ++dstbuf, ++srcbuf)
		*dstbuf = *srcbuf;
	return (dst);
}
