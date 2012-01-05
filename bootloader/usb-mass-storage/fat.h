#include <stdint.h>

#define __packed __attribute__((__packed__))

struct fat1216_super {
	uint8_t		jmp[3];
	char		oem_name[8];
	uint16_t	bytes_per_sector;
	uint8_t		sectors_per_cluster;
	uint16_t	reserved_sectors;
	uint8_t		fat_count;
	uint16_t	rootdir_count;
	uint16_t	sector_count;
	uint8_t		media_type;
	uint16_t	sectors_per_fat;
	uint16_t	chs_s;
	uint16_t	chs_h;
	uint32_t	hidden_sectors;
	uint32_t	sector_count32;
	uint8_t		drive_number;
	uint8_t		reserved1;
	uint8_t		extended_sig;
	uint32_t	serial;
	char		label[11];
	char		fat_name[8];
} __packed;

struct fat16_fat {
	uint16_t media_desc;
	uint16_t eoc_marker;
	uint16_t clusters[0];
} __packed;

struct fat12_fat {
	uint8_t media_desc_eoc_marker[3];
	uint8_t clusters[0];
} __packed;

struct fat_dir_time {
	uint16_t second : 5;
	uint16_t minute : 6;
	uint16_t hour	: 5;
} __packed;

struct fat_dir_date {
        uint16_t day	: 5;
        uint16_t month	: 4;
        uint16_t year	: 7;
} __packed;

struct fat_dir_entry {
	char			name[8];
	char			ext[3];
	struct fat_dir_attr {
		uint8_t read_only	: 1;
		uint8_t hidden		: 1;
		uint8_t system		: 1;
		uint8_t volume_label	: 1;
		uint8_t dir		: 1;
		uint8_t device		: 1;
		uint8_t reserved	: 1;
	}			attr;
	uint8_t			reserved1;
	uint8_t			ctime_10ms;
	struct fat_dir_time	ctime;
	struct fat_dir_date	cdate;
	struct fat_dir_date	adate;
	uint16_t		reserved2;
	struct fat_dir_time	mtime;
	struct fat_dir_date	mdate;
	uint16_t		start_cluster;
	uint32_t		size;
} __packed;
