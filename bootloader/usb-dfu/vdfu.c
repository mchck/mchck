#include <stdio.h>

#include <usb/usb.h>
#include <usb/dfu.h>


static char fw_buf[4096];

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
        printf("setup_write: off %zd, len %zd\n", off, len);
        *buf = fw_buf;
        return (DFU_STATUS_OK);
};

static enum dfu_status
finish_write(void *buf, size_t off, size_t len)
{
        if (off + len > 65536)
                return (DFU_STATUS_errADDRESS);
        printf("finish_write: off %zd, len %zd\n", off, len);
        return (DFU_STATUS_OK);
}

static struct dfu_ctx dfu_ctx;

static void
init_usb_bootloader(int config)
{
        dfu_init(setup_write, finish_write, &dfu_ctx);
}

#include "desc.c"

int
main(void)
{
        usb_init(&dfu_device);
        vusb_main_loop();
}
