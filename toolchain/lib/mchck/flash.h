#define FLASH_SECTOR_SIZE 1024
#define FLASH_SECTION_SIZE 1024
#define FLASH_ELEM_SIZE 4

__attribute__((section(".ramtext.ftfl_submit_cmd"), long_call))
int ftfl_submit_cmd(void);
int flash_prepare_flashing(void);
int flash_erase_sector(uintptr_t);
int flash_program_section(uintptr_t, size_t);
int flash_program_sector(const char *, uintptr_t, size_t);
void *flash_get_staging_area(uintptr_t, size_t);
