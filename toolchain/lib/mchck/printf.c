#ifndef TEST_PRINTF
#include <mchck.h>
#else
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#define vfprintf my_vfprintf
#define printf my_printf
#endif

//#define PRINTF_WITH_PAD
//#define PRINTF_WITH_64BIT

#ifdef PRINTF_WITH_64BIT
#define PRINTF_VAL_T long long
#define PRINTF_FRAC_T long fract
const static unsigned long long fract_half = 0x800000000000000ULL;
#else
#define PRINTF_VAL_T int
#define PRINTF_FRAC_T fract
const static unsigned long fract_half = 0x8000000UL;
#endif


int
vfprintf(FILE *f, const char *fmt, va_list args)
{
        /* XXX ugly code ahead.  need to refactor. */

        --fmt;

next_char:
        ++fmt;

        if (*fmt == 0)
                goto out;

        if (*fmt != '%') {
plain_output:
                fputc(*fmt, f);
                goto next_char;
        }

        /* format spec! */
        ++fmt;

        int width = -1;
        int precision = -1;
        int length = 32;
        int alternate = 0;
        int sign = 0;

        int print_sign = 0;
        int base = 10;
        int digit_off = 0;
#ifdef PRINTF_WITH_PAD
        int pad = ' ';
        int pad_right = 0;
#endif
        int print_frac = 0;
        unsigned PRINTF_VAL_T val;
#ifdef PRINTF_WITH_FIXPOINT
        unsigned PRINTF_FRAC_T frac = 0;
#endif

        /* flags */
        for (;; ++fmt) {
                switch (*fmt) {
                case '%':
                        goto plain_output;
                case '#':
                        alternate = 1;
                        break;
                case '0':
#ifdef PRINTF_WITH_PAD
                        pad = '0';
#endif
                        break;
                case ' ':
                case '+':
                        print_sign = *fmt;;
                        break;
                case '-':
#ifdef PRINTF_WITH_PAD
                        pad_right = 1;
#endif
                        break;
                default:
                        goto end_flags;
                }
        }
end_flags:

        /* width */
        if (*fmt == '*') {
                width = va_arg(args, int);
                fmt++;
        } else {
                for (; *fmt >= '0' && *fmt <= '9'; ++fmt) {
                        if (width < 0)
                                width = 0;
                        width = width * 10 + *fmt - '0';
                }
        }

        /* prec */
        if (*fmt == '.') {
                ++fmt;
                if (*fmt == '*') {
                        precision = va_arg(args, int);
                        fmt++;
                } else {
                        precision = 0;
                        for (; *fmt >= '0' && *fmt <= '9'; ++fmt)
                                precision = precision * 10 + *fmt - '0';
                }
        }

        /* length */
        switch (*fmt) {
        case 'h':
                length = 16;
                ++fmt;
                if (*fmt == 'h') {
                        length = 8;
                        ++fmt;
                }
                break;
        case 'l':
                length = 32;
                ++fmt;
#ifdef PRINTF_WITH_64BIT
                if (*fmt == 'l') {
                        /* FALLTHROUGH */
        case 'j':
                        length = 64;
                        ++fmt;
                }
#endif
                break;
        case 'z':
        case 't':
                length = 32;
                ++fmt;
                break;
        }

        /* conv */
        switch (*fmt) {
        case 0:
                /* premature end */
                goto out;

        default:
                /* fail */
                fputc('%', f);
                goto plain_output;

        case 'c':
                fputc(va_arg(args, int), f);
                break;

        case 's': {
                const char *str = va_arg(args, const char *);

                for (; precision != 0 && *str != 0; --precision, ++str)
                        fputc(*str, f);
                break;
        }

        case 'd':
        case 'i':
                sign = 1;
                goto conv_int;
        case 'o':
                base = 8;
                goto conv_int;
        case 'p':
                alternate = 1;
                length = 32;
                /* FALLTHROUGH */
        case 'x':
        case 'X':
                base = 16;
                if (*fmt == 'X')
                        digit_off = 'A' - '0' - 10;
                else
                        digit_off = 'a' - '0' - 10;
                /* FALLTHROUGH */
        case 'u':
                goto conv_int;

#ifdef PRINTF_WITH_FIXPOINT
        case 'k': {
                long accum fpval;

                switch (length) {
                case 64:
                        fpval = va_arg(args, long accum);
                        break;
                case 16:
                        fpval = va_arg(args, short accum);
                        break;
                default:
                        fpval = va_arg(args, accum);
                        break;
                }
                sign = 1;
                if (fpval < 0) {
                        fpval = -fpval;
                        print_sign = '-';
                }
                val = fpval;
                frac = fpval;
                goto conv_frac;
        }

        case 'K': {
                long unsigned accum fpval;

                switch (length) {
                case 64:
                        fpval = va_arg(args, long unsigned accum);
                        break;
                case 16:
                        fpval = va_arg(args, short unsigned accum);
                        break;
                default:
                        fpval = va_arg(args, unsigned accum);
                        break;
                }
                val = fpval;
                frac = fpval;
                goto conv_frac;
        }

        case 'r': {
                long fract fpval;

                switch (length) {
                case 64:
                        fpval = va_arg(args, long fract);
                        break;
                case 16:
                        fpval = va_arg(args, short fract);
                        break;
                default:
                        fpval = va_arg(args, fract);
                        break;
                }
                sign = 1;
                if (fpval < 0) {
                        fpval = -fpval;
                        print_sign = '-';
                }
                val = fpval;
                frac = fpval;
                goto conv_frac;
        }

        case 'R': {
                unsigned long fract fpval;

                switch (length) {
                case 64:
                        fpval = va_arg(args, unsigned long fract);
                        break;
                case 16:
                        fpval = va_arg(args, unsigned short fract);
                        break;
                default:
                        fpval = va_arg(args, unsigned fract);
                        break;
                }
                val = fpval;
                frac = fpval;
                goto conv_frac;
        }
#endif  /* PRINTF_WITH_FIXPOINT */

        }

        goto next_char;

#ifdef PRINTF_WITH_FIXPOINT
conv_frac:
        if (precision < 0)
                precision = 6;
        print_frac = 1;
        goto print_int;
#endif

conv_int:
        if (length == 64)
                val = va_arg(args, uint64_t);
        else
                val = va_arg(args, uint32_t);
        /* sign extend if required */
        if (sign) {
                switch (length) {
                case 8:
                        val = (int8_t)val;
                        break;
                case 16:
                        val = (int16_t)val;
                        break;
                case 32:
                        val = (int32_t)val;
                        break;
                }
                if ((PRINTF_VAL_T)val < 0) {
                        val = -(PRINTF_VAL_T)val;
                        print_sign = '-';
                }
        }

        switch (length) {
        case 8:
                val = (uint8_t)val;
                break;
        case 16:
                val = (uint16_t)val;
                break;
        case 32:
                val = (uint32_t)val;
                break;
        }

#ifdef PRINTF_WITH_FIXPOINT
print_int:;
#endif

        /* determine first digit */
        unsigned PRINTF_VAL_T scale = 1;

#ifdef PRINTF_WITH_PAD
        if (precision < 0)
                precision = 1;
#endif

        int print_digits = 1;
        while (val / base >= scale) {
                scale *= base;
                ++print_digits;
        }

#if defined(PRINTF_WITH_PAD)
        int print_width;

        if (print_frac)
                print_width = precision + print_digits;
        else
                print_width = precision > print_digits ? precision : print_digits;

        if (alternate && base == 8 && val != 0 && precision <= print_digits)
                print_width = precision = print_digits + 1;

        if (val == 0 && precision == 0 && !(alternate && base == 8)) {
                scale = 0;
                print_width = 0;
                print_digits = 0;
        }

        /* will print sign */
        if (print_sign)
                ++print_width;

        /* will print decimal point */
        if (print_frac && (precision > 0 || alternate))
                ++print_width;

        /* will print 0x prefix */
        if (alternate && val != 0) {
                switch (base) {
                case 16:
                        print_width += 2;
                        break;
                }
        }

        if (pad == '0') {
                if (print_sign)
                        fputc(print_sign, f);
                if (alternate && val != 0) {
                        switch (base) {
                        case 16:
                                fputc('0', f);
                                fputc('x', f);
                                break;
                        }
                }
        }

        if (!pad_right) {
                while (print_width < width) {
                        fputc(pad, f);
                        ++print_width;
                }
        }

        if (pad != '0')
#endif
        {
                if (print_sign)
                        fputc(print_sign, f);
                if (alternate && val != 0) {
                        switch (base) {
                        case 16:
                                fputc('0', f);
                                fputc('x', f);
                                break;
                        }
                }
        }
#ifdef PRINTF_WITH_PAD
        while (!print_frac && print_digits < precision) {
                fputc('0', f);
                ++print_digits;
        }
#endif
        while (scale > 0) {
                int digit = val / scale;

                if (digit > 9)
                        digit += digit_off;
                digit += '0';
                fputc(digit, f);
                val %= scale;
                scale /= base;
        }

        if (!print_frac || (precision <= 0 && !alternate))
                goto pad_right;

#ifdef PRINTF_WITH_FIXPOINT
        fputc('.', f);
        for (; precision > 0; --precision) {
                int digit = 10 * frac;

                digit += '0';
                fputc(digit, f);
                frac *= 10;
        }
#endif

pad_right:
#ifdef PRINTF_WITH_PAD
        if (pad_right) {
                while (print_width < width) {
                        fputc(' ', f);
                        ++print_width;
                }
        }
#endif
        goto next_char;

out:
        return (0);
}

int
printf(const char *fmt, ...)
{
        va_list args;
        int ret;

        va_start(args, fmt);
        ret = vfprintf(stdout, fmt, args);
        va_end(args);

        return (ret);
}

#ifdef TEST_PRINTF
#undef vfprintf
#undef printf
int
main(void)
{
#define T(fmt, ...) {                                                   \
                my_printf("my:  %s: " fmt "\n", fmt, ## __VA_ARGS__);   \
                printf("org: %s: " fmt "\n", fmt, ## __VA_ARGS__);      \
        } while (0)

        T("test");
        T("%d", 1234);
        T("%d", -1234);
        T("%#x", 0x1234);
        T("%#X", -0x1234);
        T("%#o", 01234);
        T("%llo", 01234567012345670123456LL);
        T("%p", main);
        T("%c", 'O');
        T("%s", "testing!");
        T("%.4s", "testing!");
        T("%u", -2);
        T("%hhu", -2);
        T("%hu", -2);
        T("%lu", -2L);
        T("%llu", -2LL);
        T("%hhd", 255);
        T("%#.4x", 23);
        T("%#.30x", 23);
        T("%30x.", 23);
        T("%#30x.", 23);
        T("%0*d", 10, -5);
        T("%-*d.", 10, -5);
        T("%10d.", -5);
        T("%0+*d.", 10, 5);
        T("%10.15d.", 123);
        T("%15.10d.", 123);
        T("%015d.", 123);
        T("%2.0u.", 0);
        T("%#-10.5o.", 12);
        T("%#10.5o", 12);
        T("%#10.5x", 12);
        T("%#-5.o.", 0);
        T("%#-5.x.", 0);
        T("%5.u.", 0);
        T("%#-10x.", 5);
        T("%#-10o.", 5);
        T("%#-10o.", 8);
        T("%+.d.", 0);
        return (0);
}
#endif
