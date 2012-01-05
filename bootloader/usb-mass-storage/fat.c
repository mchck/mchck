#include <stdio.h>
#include <unistd.h>

#include "fat.h"

static struct fat1216_super super = {
        .jmp = { 0x90, 0x90, 0x90 },
        .oem_name = "MCHCK   ",
        .bytes_per_sector = 512,
        .sectors_per_cluster = 128,
        .reserved_sectors = 1,
        .fat_count = 1,
        .rootdir_count = 512 / sizeof(struct fat_dir_entry),
        .sector_count = /* super */ 1 + /* fat */ 1 + /* dir */ 1 + /* data */ 128,
        .media_type = 0xF8,
        .sectors_per_fat = 1,
        .chs_s = 1,
        .chs_h = 1,
        .hidden_sectors = 0,
        .sector_count32 = 0,
        .drive_number = 0,
        .reserved1 = 0,
        .extended_sig = 0x29,
        .serial = 0x2c2c2c2c,
        .label = "MC HCK BOOT",
        .fat_name = "FAT12   ",
};

static struct fat12_fat fat = { { 0xF8, 0xff, 0xff } };

static struct fat_dir_entry dir = {
        .name = "MC HCK B",
        .ext = "OOT",
        .attr = { .reserved = 0,
                  .device = 0,
                  .dir = 0,
                  .volume_label = 1,
                  .system = 0,
                  .hidden = 0,
                  .read_only = 0,
        },
        .reserved1 = 0,
        .ctime_10ms = 0,
        .ctime = { .hour = 0,
                   .minute = 0,
                   .second = 0,
        },
        .cdate = { .year = 23,
                   .month = 5,
                   .day = 23,
        },
        .adate = { .year = 0,
                   .month = 0,
                   .day = 0,
        },
        .reserved2 = 0,
        .mtime = { .hour = 0,
                   .minute = 0,
                   .second = 0,
        },
        .mdate = { .year = 23,
                   .month = 5,
                   .day = 23,
        },
        .start_cluster = 0,
        .size = 0,
};


int
main(void)
{
        write(1, &super, sizeof(super));
        for (int i = sizeof(super); i < 512; ++i) {
                char zero = 0;
                write(1, &zero, 1);
        }

        write(1, &fat, sizeof(fat));
        for (int i = sizeof(fat); i < 512; ++i) {
                char zero = 0;
                write(1, &zero, 1);
        }

	write(1, &dir, sizeof(dir));
        for (int i = sizeof(dir); i < 512; ++i) {
                char zero = 0;
                write(1, &zero, 1);
        }
 
        for (int i = 0; i < (super.bytes_per_sector *
			     (super.sector_count - 3)); ++i) {
                char zero = 0;
                write(1, &zero, 1);
        }
 
        return (0);
}
