#include <mchck.h>

static FILE _stdout_instance;

FILE *stdout = &_stdout_instance;

void
fputc(int c, FILE *f)
{
        crit_enter();
        f->outbuf[f->outbuf_pos++] = c;
        /* flush on newline for now */
        if (c == '\n' || f->outbuf_pos >= sizeof(f->outbuf)) {
                f->ops->write(f->outbuf, f->outbuf_pos, f->ops_data);
                f->outbuf_pos = 0;
        }
        crit_exit();
}
