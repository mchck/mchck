#include <mchck.h>

static FILE _stdout_instance;

FILE *stdout = &_stdout_instance;

void
fputc(int c, FILE *f)
{
        crit_enter();
        f->outbuf[f->outbuf_head] = c;
        f->outbuf_head = (f->outbuf_head + 1) % sizeof(f->outbuf);

        /* number of buffered characters */
        size_t buffered = f->outbuf_head - f->outbuf_tail;
        if (buffered < 0)
                buffered += sizeof(f->outbuf) - 1;

        /* flush on newline or if more than half full */
        if (c == '\n' || buffered > sizeof(f->outbuf) / 2) {
                size_t n;
                if (f->outbuf_head > f->outbuf_tail) {
                        n = buffered;
                } else {
                        n = sizeof(f->outbuf) - f->outbuf_tail;
                }
                size_t written = f->ops->write(&f->outbuf[f->outbuf_tail],
                                               n, f->ops_data);
                f->outbuf_tail = (f->outbuf_tail + written) % sizeof(f->outbuf);
        }
        crit_exit();
}
