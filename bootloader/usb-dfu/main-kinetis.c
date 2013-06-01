#include <mchck.h>

#include <usb/usb.h>
#include <usb/dfu.h>
#include "flash.h"

/**
 * Unfortunately we can't DMA directly to FlexRAM, so we'll have to stage here.
 */
static char staging[FLASH_SECTOR_SIZE];

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
        if (len > sizeof(staging))
                return (DFU_STATUS_errADDRESS);
        *buf = staging;
        return (DFU_STATUS_OK);
}

static enum dfu_status
finish_write(void *buf, size_t off, size_t len)
{
        void *target;

        if (len == 0)
                return (DFU_STATUS_OK);

        target = flash_get_staging_area(off + (uintptr_t)&_app_rom, len);
        if (!target)
                return (DFU_STATUS_errADDRESS);
        memcpy(target, buf, len);
        if (flash_program_sector(off + (uintptr_t)&_app_rom, len) != 0)
                return (DFU_STATUS_errADDRESS);
        return (DFU_STATUS_OK);
}

void
main(void)
{
        SIM.sopt2.usbsrc = 1;    /* usb from mcg */
        flash_prepare_flashing();

        dfu_start(setup_write, finish_write);
        for (;;) {
                usb_intr();
        }
}
