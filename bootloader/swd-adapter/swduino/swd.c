/**
 * For now the protocol is a simple bitbang protocol.
 *
 * Prototcol messages:
 * - version
 * 0x3f...
 * request: ?SWD?
 * respnse: !SWD1
 * only after this "handshake", other output functions are enabled.
 *
 * - write_word
 * 0x90, 4 bytes payload (first byte, LSB transmitted first)
 * - write_bits
 * 0xa0 + (bitcount - 1), 1 byte payload (LSB transmitted first)
 * - read_word
 * 0x10 => 4 bytes
 * - read_bits
 * 0x20 + (bitcount - 1) => 1 byte
 * - cycle clock (silent read)
 * 0x28 + (bitcount - 1)
 */

#include "swd.h"

static enum {
        PIN_UNCONFIGURED = 0,
        PIN_CONFIGURED,
} iostate = PIN_UNCONFIGURED;

static const uint8_t magic[] = "?SWD?";
static const uint8_t version[] = "!SWD1";


static void
write_bits(uint8_t buf, size_t bits)
{
        pin_configure(SWD_DIO_PIN, SWD_MODE_OUTPUT);
        for (; bits > 0; --bits, buf >>= 1) {
                pin_write(SWD_CLK_PIN, 0);
                pin_write(SWD_DIO_PIN, buf & 1);
                pin_write(SWD_CLK_PIN, 1);
        }
}

static void
read_bits(size_t bits, int silent)
{
        uint8_t val = 0;

        pin_configure(SWD_DIO_PIN, SWD_MODE_INPUT);
        size_t bit;
        for (bit = 0; bit < bits; ++bit) {
                pin_write(SWD_CLK_PIN, 0);
                val |= !!pin_read(SWD_DIO_PIN) << bit;
                pin_write(SWD_CLK_PIN, 1);
        }
        if (!silent)
                reply_write(&val, 1);
}

static void
configure_pins(int on)
{
        pin_configure(SWD_DIO_PIN, SWD_MODE_INPUT);
        pin_configure(SWD_CLK_PIN, on ? SWD_MODE_OUTPUT : SWD_MODE_INPUT);
        iostate = on ? PIN_CONFIGURED : PIN_UNCONFIGURED;
}


const uint8_t *
process_command(const uint8_t *buf, size_t len, int *outpipe_full)
{
        signal_led();

        for (; len > 0;) {
                size_t i;
                uint8_t cmd = *buf;

                ++buf; --len;

                if (iostate == PIN_UNCONFIGURED)
                        goto test_magic;

                switch (cmd) {
                case 0x10:       /* read word */
                        if (!outpipe_space(4))
                                goto outpipe_full;
                        for (i = 4; i > 0; --i)
                                read_bits(8, 0);
                        break;

                case 0x20 ... 0x27: /* read bits */
                        if (!outpipe_space(1))
                                goto outpipe_full;
                        read_bits((cmd & 7) + 1, 0);
                        break;

                case 0x28 ... 0x2f: /* cycle clock */
                        read_bits((cmd & 7) + 1, 1);
                        break;

                case 0x90:       /* write word */
                        if (len < 4)
                                goto need_more_data;
                        for (i = 4; i > 0; --i, ++buf, --len)
                                write_bits(*buf, 8);
                        break;

                case 0xa0 ... 0xa7: /* write bits */
                        if (len < 1)
                                goto need_more_data;
                        write_bits(*buf, (cmd & 7) + 1);
                        ++buf; --len;
                        break;

                default:
                case '?':        /* handshake? */
test_magic:
                        configure_pins(0);
                        if (len < sizeof(magic) - 1 /* NUL byte */ - 1 /* we already skipped one */)
                                goto need_more_data;
                        if (memcmp(buf - 1, magic, sizeof(magic) - 1) != 0)
                                continue;
                        /* We found a handshake! */
                        buf += sizeof(magic) - 1 /* NUL byte */ - 1 /* we already skipped one */;
                        len -= sizeof(magic) - 1 - 1;
                        configure_pins(1);
                        reply_write(version, sizeof(version) - 1);
                        break;
                }
        }

        return (buf);

outpipe_full:
        if (outpipe_full != NULL)
                *outpipe_full = 1;

need_more_data:
        /* we skipped one, now go back */
        return (buf - 1);
}

int
process_data(const uint8_t *data, size_t len)
{
        static uint8_t buf[10];
        static size_t buflen;

        int outpipe_full = 0;

        while (buflen + len > 0) {
                size_t copylen = sizeof(buf) - buflen;

                if (len < copylen)
                        copylen = len;
                memcpy(buf + buflen, data, copylen);
                len -= copylen;
                buflen += copylen;
                data += copylen;

                outpipe_full = 0;
                const uint8_t *p = process_command(buf, buflen, &outpipe_full);
                if (p == buf)
                        break;
                buflen -= p - buf;
                memcpy(buf, p, buflen);
        }

        return (outpipe_full);
}
