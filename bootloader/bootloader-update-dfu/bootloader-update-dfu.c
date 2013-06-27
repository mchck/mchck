#include <mchck.h>

#include <usb/usb.h>
#include <usb/dfu.h>

/**
 * We know what we're doing, part 1.
 */
extern uint32_t flash_ALLOW_BRICKABLE_ADDRESSES;

/**
 * We buffer the whole bootloader here before we go on to flash it.
 */
static char staging[FLASH_SECTOR_SIZE * 3];

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
        if (len > sizeof(staging))
                return (DFU_STATUS_errADDRESS);
        *buf = staging + off;
        return (DFU_STATUS_OK);
}

static enum dfu_status
finish_write(void *buf, size_t off, size_t len)
{
        void *target;

        /* buffer all while we are not done */
        if (len != 0)
                return (DFU_STATUS_OK);

        struct FTFL_CONFIG_t *flashconfig = (void *)&staging[(uintptr_t)&FTFL_CONFIG];

        /* Make sure we don't brick ourselves */
        if (flashconfig->fsec.sec != FTFL_FSEC_SEC_UNSECURE)
                return (DFU_STATUS_errFILE);

        /* We know what we're doing, part 2. */
        flash_ALLOW_BRICKABLE_ADDRESSES = 0x00023420;

        for (size_t pos = 0; pos < sizeof(staging); pos += FLASH_SECTOR_SIZE) {
                target = flash_get_staging_area(pos, FLASH_SECTOR_SIZE);
                if (!target)
                        return (DFU_STATUS_errADDRESS);
                memcpy(target, staging + pos, FLASH_SECTOR_SIZE);
                if (flash_program_sector(pos, FLASH_SECTOR_SIZE) != 0)
                        return (DFU_STATUS_errADDRESS);
        }
        return (DFU_STATUS_OK);
}

static void
onboard_led_morse_raw(const char *signs)
{
        for (const char *p = signs; *p != 0; ++p) {
                const int scale = 1000000;
                int dur;

                onboard_led(1);
                switch (*p) {
                case '.':
                        dur = 1;
                        break;
                case '-':
                        dur = 3;
                        break;
                default:
                        onboard_led(0);
                        /**
                         * Actually 5, but there is
                         * one space anyways.
                         */
                        dur = 4;
                        break;
                }
                for (volatile int i = 0; i < dur * scale; ++i)
                        /* NOTHING */;
                onboard_led(0);
                for (volatile int i = 0; i < scale; ++i)
                        /* NOTHING */;
        }
}

void
main(void)
{
        /**
         * Make sure our idea of the bootloader size matches reality.
         */
        if (sizeof(staging) != (uintptr_t)&_app_rom) {
                /* Bad - abort */
                for (;;) {
                        onboard_led_morse_raw("...---... ");
                }
        }

        onboard_led(1);

        SIM.sopt2.usbsrc = 1;    /* usb from mcg */
        flash_prepare_flashing();

        dfu_start(setup_write, finish_write);
        for (;;) {
                usb_intr();
        }
}
