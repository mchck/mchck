#define SPIFLASH_MFGID_WINBOND 0x9F
#define SPIFLASH_MEMTYPE_WINBOND_FLASH 0x40
#define SPIFLASH_WINBOND_SIZE_1MB 0x14

typedef void (spiflash_info_cb)(void *cbdata, uint8_t mfg_id, uint8_t memtype, uint8_t capacity);
typedef void (spiflash_status_cb)(void *cbdata, uint8_t status);

extern void
spiflash_pins_init(void);

extern int
spiflash_get_id(spiflash_info_cb cb, void *cbdata);

extern int
spiflash_get_status(spiflash_status_cb cb, void *cbdata);

extern int
spiflash_program_page(uint32_t addr, const uint8_t *src, uint8_t len, spi_cb cb, void *cbdata);

extern int
spiflash_read_page(uint8_t *dest, uint32_t addr, uint32_t len, spi_cb cb, void *cbdata);

extern int
spiflash_erase_sector(uint32_t addr, spi_cb cb, void *cbdata);

