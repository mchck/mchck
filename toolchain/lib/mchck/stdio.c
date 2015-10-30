#include <mchck.h>

static FILE _stdout_instance;

FILE *stdout = &_stdout_instance;

void
fflush(FILE *f)
{
        size_t n;

        while (f->outbuf_head != f->outbuf_tail) {
                if (f->outbuf_head >= f->outbuf_tail) {
                        n = f->outbuf_head - f->outbuf_tail;
                } else {
                        n = sizeof(f->outbuf) - f->outbuf_tail;
                }

                if (n == 0)
                        return;

                size_t written = f->ops->write(&f->outbuf[f->outbuf_tail], n, f->ops_data);
                f->outbuf_tail = (f->outbuf_tail + written) % sizeof(f->outbuf);
                if (written != n)
                        break;
        }
}

static size_t
buf_count(FILE *f)
{
        /* number of buffered characters */
        size_t buffered = f->outbuf_head - f->outbuf_tail;
        if (buffered < 0)
                buffered += sizeof(f->outbuf) - 1;

        return (buffered);
}

void
fputc(int c, FILE *f)
{
        crit_enter();
        f->outbuf[f->outbuf_head] = c;
        f->outbuf_head = (f->outbuf_head + 1) % sizeof(f->outbuf);

        /* flush on newline or if more than half full */
        if (f->ops && (c == '\n' || buf_count(f) > sizeof(f->outbuf) / 2))
                fflush(f);
        crit_exit();
}

struct buffer_file_ctx {
        char *buf;
        size_t buflen;
        size_t len;
};

#define MIN(a,b) ((a) < (b) ? (a) : (b))

static size_t
buffer_file_write(const uint8_t *buf, size_t len, void *ops_data)
{
        struct buffer_file_ctx *ctx = ops_data;
        size_t n = MIN(ctx->buflen - ctx->len, len);
        memcpy(&ctx->buf[ctx->len], buf, n);
        ctx->len += n;
        return n;
}

struct _stdio_file_ops buffer_opts = {
        .init = NULL,
        .write = buffer_file_write
};

static void
buffer_file_init(FILE *f, struct buffer_file_ctx *ctx,
                 char *buf, size_t buflen)
{
        ctx->buf = buf;
        ctx->buflen = buflen;
        ctx->len = 0;
        f->ops_data = ctx;
}

int vsnprintf(char *buf, size_t n, const char *fmt, va_list args)
{
        FILE f;
        struct buffer_file_ctx ctx;
        buffer_file_init(&f, &ctx, buf, n);
        vfprintf(&f, fmt, args);
        // null terminate
        buf[ctx.len] = 0;
        return ctx.len - 1;
}

int snprintf(char *buf, size_t n, const char *fmt, ...)
{
        va_list args;
        int ret;

        va_start(args, fmt);
        ret = vsnprintf(buf, n, fmt, args);
        va_end(args);
        return ret;
}
