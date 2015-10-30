/*
 * We define these ourselves as linking against newlib's definitions leads to
 * a 1kB impure_data global being brought in to the image.
 */

// Enable this and build on host for tests
//#define TEST

#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#ifdef TEST
#include <stdio.h>
#endif

static int
digit_value(char c, int base)
{
        // 0-9 digit
        if ((c >= '0') && (c <= '9') && (c - '0' < base))
                return c - '0';
        // a-z digit
        else if ((c >= 'a') && (c <= 'z') && (c - 'a' + 10 < base))
                return c - 'a' + 10;
        // A-Z digit
        else if ((c >= 'A') && (c <= 'Z') && (c - 'A' + 10 < base))
                return c - 'A' + 10;
        else
                return -1;
}
        
unsigned long
strtoul(const char *p, char **endptr, int base)
{
	long v = 0;
        bool negate = false;
        bool overflow = false;

        /* explicit cast to work around "array subscript has type
         * 'char'" warnings from gcc
         */
	while (isspace((signed char) p[0]))
		p++;

        // strtoul should return the unsigned representation of the signed scan result
        if (p[0] == '-') {
                negate = true;
                p++;
        }

        // Move past initial "0x"
	if (((base == 16) || (base == 0)) &&
	    ((p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X')))) {
		p += 2;
		base = 16;

        // Figure out base otherwise
	} else if (base == 0) {
		if (p[0] == '0')
			base = 8;
		else
			base = 10;
	}

	for (;;) {
                int d = digit_value(*p, base);

                // invalid character
                if (d < 0) {
                        break;
                } else {
                        // ensure that v * base + d < ULONG_MAX
                        if ((ULONG_MAX - d) / base < v)
                                overflow = true;
                        if (!overflow)
                                v = v * base + d;
                }
		p++;
	}

	if (endptr)
                *endptr = (char*) p;

        if (overflow)
                return ULONG_MAX;
        else
                return (unsigned long) (negate ? -v : v);
}

long
strtol(const char *p, char **endptr, int base)
{
        unsigned long v = strtoul(p, endptr, base);
        return (long) v;
}

#ifdef TEST
int main(int argc, char **argv)
{
        char *endptr;
        int base = atoi(argv[1]);
        unsigned long v = strtoul(argv[2], &endptr, base);
        printf("%lu\n", v);
}
#endif
