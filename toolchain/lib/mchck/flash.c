#include <mchck.h>

uint32_t flash_ALLOW_BRICKABLE_ADDRESSES;

static void
flash_prepare_cmd(enum FTFL_FCMD cmd)
{
        FTFL.fccob.generic.fcmd = cmd;
        for (int i=0; i<8; i++)
                FTFL.fccob.generic.data_be[i] = 0;
}

/* This will have to live in SRAM. */
__attribute__((section(".ramtext.ftfl_submit_cmd"), long_call))
int
ftfl_submit_cmd(void)
{
        FTFL.fstat.raw = ((struct FTFL_FSTAT_t){
                        .ccif = 1,
                                .rdcolerr = 1,
                                .accerr = 1,
                                .fpviol = 1
                                }).raw;
        struct FTFL_FSTAT_t stat;
        while (!(stat = FTFL.fstat).ccif)
                /* NOTHING */; /* XXX maybe WFI? */
        stat.ccif = 0;
        return (stat.raw != 0);
}

int
flash_prepare_flashing(void)
{
        /* switch to FlexRAM */
        if (!FTFL.fcnfg.ramrdy) {
                FTFL.fccob.set_flexram.fcmd = FTFL_FCMD_SET_FLEXRAM;
                FTFL.fccob.set_flexram.flexram_function = FTFL_FLEXRAM_RAM;
                return (ftfl_submit_cmd());
        }
        return (0);
}

int
flash_erase_sector(uintptr_t addr)
{
        if (addr < (uintptr_t)&_app_rom &&
                flash_ALLOW_BRICKABLE_ADDRESSES != 0x00023420)
                return (-1);
        FTFL.fccob.erase.fcmd = FTFL_FCMD_ERASE_SECTOR;
        FTFL.fccob.erase.addr = addr;
        return (ftfl_submit_cmd());
}

int
flash_program_section(uintptr_t addr, size_t len)
{
        FTFL.fccob.program_section.fcmd = FTFL_FCMD_PROGRAM_SECTION;
        FTFL.fccob.program_section.addr = addr;
        FTFL.fccob.program_section.num_elems = len / FLASH_ELEM_SIZE;
        return (ftfl_submit_cmd());
}

int
flash_program_sector(const char *buf, uintptr_t addr, size_t len)
{
        if (len != FLASH_SECTOR_SIZE)
                return 2;
        if ((addr & (FLASH_SECTOR_SIZE - 1)) != 0)
                return 3;
        if (flash_erase_sector(addr))
                return 4;

        int ret = 0;
        for (int i = FLASH_SECTOR_SIZE / FLASH_SECTION_SIZE; i > 0; --i) {
                memcpy(flash_get_staging_area(addr, FLASH_SECTION_SIZE),
                       buf,
                       FLASH_SECTION_SIZE);
                ret = ret || flash_program_section(addr, FLASH_SECTION_SIZE);
                buf += FLASH_SECTION_SIZE;
                addr += FLASH_SECTION_SIZE;
        }

        return ret;
}

void *
flash_get_staging_area(uintptr_t addr, size_t len)
{
        if ((addr & (FLASH_SECTION_SIZE - 1)) != 0 ||
            len != FLASH_SECTION_SIZE)
                return (NULL);
        return (FlexRAM);
}

int
flash_set_partitioning(enum FTFL_FLEXNVM_PARTITION flexnvm_partition,
                       enum FTFL_EEPROM_SIZE eeprom_size)
{
        /* first verify that the IFR is clear */
        flash_prepare_cmd(FTFL_FCMD_READ_RESOURCE);
        FTFL.fccob.read_resource.resource_select = FTFL_RESOURCE_IFR;
        FTFL.fccob.read_resource.addr = 0xfc;
        ftfl_submit_cmd();
        if (FTFL.fccob.read_resource.data != 0xffffffff)
                return 1;

        /* setup the PROGRAM_PARTITION command */
        flash_prepare_cmd(FTFL_FCMD_PROGRAM_PARTITION);
        FTFL.fccob.program_partition.flexnvm_partition = flexnvm_partition;
        FTFL.fccob.program_partition.eeprom_size = eeprom_size;
        ftfl_submit_cmd();
        if (FTFL.fstat.accerr)
                return 2;
        else if (FTFL.fstat.mgstat0)
                return 3;
        else
                return 0;
}
