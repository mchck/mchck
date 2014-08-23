#include <mchck.h>

#include "dfu.desc.h"


/**
 * Unfortunately we can't DMA directly to FlexRAM, so we'll have to stage here.
 */
static char staging[FLASH_SECTOR_SIZE];

static enum dfu_status
setup_write(size_t off, size_t len, void **buf)
{
        static int last = 0;

        if (len > sizeof(staging))
                return (DFU_STATUS_errADDRESS);
        /**
         * We only allow the last write to be less than one sector size.
         */
        if (off == 0)
                last = 0;
        if (last && len != 0)
                return (DFU_STATUS_errADDRESS);
        if (len != FLASH_SECTOR_SIZE) {
                last = 1;
                memset(staging, 0xff, sizeof(staging));
        }

        *buf = staging;
        return (DFU_STATUS_OK);
}

static enum dfu_status
finish_write(void *buf, size_t off, size_t len)
{
        if (len == 0)
                return (DFU_STATUS_OK);

        if (flash_program_sector(buf, off + (uintptr_t)&_app_rom, FLASH_SECTOR_SIZE) != 0)
                return (DFU_STATUS_errADDRESS);
        return (DFU_STATUS_OK);
}


static struct dfu_ctx dfu_ctx;

void
init_usb_bootloader(int config)
{
        dfu_init(setup_write, finish_write, &dfu_ctx);
}

void
main(void)
{
        flash_prepare_flashing();

        usb_init(&dfu_device);
#ifdef SHORT_ISR
        for (;;) {
                usb_poll();
        }
#else
        sys_yield_for_frogs();
#endif
}

__attribute__((noreturn))
static inline void
jump_to_app(uintptr_t addr)
{
        /* addr is in r0 */
        __asm__("ldr sp, [%[addr], #0]\n"
                "ldr pc, [%[addr], #4]"
                :: [addr] "r" (addr));
        /* NOTREACHED */
        __builtin_unreachable();
}

void
Reset_Handler(void)
{
	/**
         * Disable Watchdog.  We spend too much time in here comparing
         * the loader magic, which will lead to a watchdog
         * configuration timeout.
         */
	WDOG_UNLOCK = 0xc520;
	WDOG_UNLOCK = 0xd928;
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
        /* XXX maybe re-enable watchdog later? */

        /**
         * We treat _app_rom as pointer to directly read the stack
         * pointer and check for valid app code.  This is no fool
         * proof method, but it should help for the first flash.
         */
        if (RCM.srs0.pin ||
            _app_rom == 0xffffffff ||
            memcmp(&VBAT, sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic)) == 0) {
                extern void Default_Reset_Handler(void);

                memset(&VBAT, 0, sizeof(VBAT));
                Default_Reset_Handler();
        } else {
                uint32_t addr = (uintptr_t)&_app_rom;
                SCB_VTOR = addr;   /* relocate vector table */
                jump_to_app(addr);
        }
}
