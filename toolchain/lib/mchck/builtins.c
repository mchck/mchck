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

int
memcmp(const void *a, const void *b, size_t len)
{
	const uint8_t *ap = a, *bp = b;
	int val = 0;

	for (; len > 0 && (val = *ap - *bp) == 0; --len, ++ap, ++bp)
		/* NOTHING */;
	return (val);
}

void *
memchr(const void *addr, int val, size_t len)
{
	const uint8_t *buf = addr;

	for (; len > 0; --len, ++buf) {
		if (*buf == val)
			return ((void *)buf);
	}
	return (NULL);
}

size_t
strlen(const char *str)
{
	size_t len = 0;

	while (*str != 0) {
		++str;
		++len;
	}
	return (len);
}

char *
strchr(const char *str, int chr)
{
	for (; *str != 0; ++str) {
		if (*str == chr)
			return ((char *)str);
	}

	return (NULL);
}

int
strcmp(const char *a, const char *b)
{
	for (; *a != 0 && *b != 0; ++a, ++b) {
		if (*a != *b)
			break;
	}

	return (*a - *b);
}

int
strncmp(const char *a, const char *b, size_t len)
{
	for (; *a != 0 && *b != 0 && len != 0; ++a, ++b, --len) {
		if (*a != *b)
			break;
	}

	if (len == 0)
		return (0);

	return (*a - *b);
}
