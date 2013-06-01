#include <mchck.h>

#include <usb/usb.h>
#include <usb/dfu.h>

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
        SIM.sopt2.usbsrc = 1;    /* usb from mcg */

        dfu_start(setup_write, finish_write);

        for (;;) {
                usb_intr();
        }
}
