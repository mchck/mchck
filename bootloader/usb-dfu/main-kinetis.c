#include "dfu.h"

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
}

static enum dfu_status
finish_write(size_t off, size_t len)
{
}

void
main(void)
{
        dfu_start(setup_write, finish_write);
}
