#include <mchck.h>

#include <usb/usb.h>
#include <usb/dfu.h>

#include "desc.h"


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
        void *target;

        if (len == 0)
                return (DFU_STATUS_OK);

        target = flash_get_staging_area(off + (uintptr_t)&_app_rom, FLASH_SECTOR_SIZE);
        if (!target)
                return (DFU_STATUS_errADDRESS);
        memcpy(target, buf, len);
        if (flash_program_sector(off + (uintptr_t)&_app_rom, FLASH_SECTOR_SIZE) != 0)
                return (DFU_STATUS_errADDRESS);
        return (DFU_STATUS_OK);
}


static struct dfu_ctx dfu_ctx;

static void
init_usb_bootloader(int config)
{
        dfu_init(setup_write, finish_write, &dfu_ctx);
}

const static struct usb_desc_dev_t dfu_dev_desc = {
        .bLength = sizeof(struct usb_desc_dev_t),
        .bDescriptorType = USB_DESC_DEV,
        .bcdUSB = { .maj = 2 },
        .bDeviceClass = USB_DEV_CLASS_SEE_IFACE,
        .bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,
        .bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,
        .bMaxPacketSize0 = EP0_BUFSIZE,
        .idVendor = 0x2323,
        .idProduct = 1,
        .bcdDevice = { .raw = 0 },
        .iManufacturer = 1,
        .iProduct = 2,
        .iSerialNumber = 0,
        .bNumConfigurations = 1,
};

const struct usb_desc_string_t * const dfu_string_descs[] = {
        USB_DESC_STRING_LANG_ENUS,
        USB_DESC_STRING(u"mchck.org"),
        USB_DESC_STRING(u"MC HCK bootloader"),
        NULL
};


const static struct usbd_config dfu_config = {
        .init = init_usb_bootloader,
        .desc = &usb_desc_config1.config, /* from desc.c */
        .function = { &dfu_function, NULL },
};

const struct usbd_device dfu_device = {
        .dev_desc = &dfu_dev_desc,
        .string_descs = dfu_string_descs,
        .configs = { &dfu_config, NULL }
};

void
main(void)
{
        SIM.sopt2.usbsrc = 1;    /* usb from mcg */
        flash_prepare_flashing();

        usb_init(&dfu_device);
        for (;;) {
                usb_intr();
        }
}

__attribute__((noreturn))
inline void
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
         * We treat _app_rom as pointer to directly read the stack
         * pointer and check for valid app code.  This is no fool
         * proof method, but it should help for the first flash.
         */
        if (RCM.srs0.pin || _app_rom == 0xffffffff) {
                extern void Default_Reset_Handler(void);
                Default_Reset_Handler();
        } else {
                uint32_t addr = (uintptr_t)&_app_rom;
                SCB_VTOR = addr;   /* relocate vector table */
                jump_to_app(addr);
        }
}
