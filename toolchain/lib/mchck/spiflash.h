typedef void (spi_status_cb)(void *cbdata, uint8_t status);

extern void
spiflash_pins_init(void);

extern void
spiflash_is_present(spi_status_cb cb, void *cbdata);

extern void
spiflash_get_status(spi_status_cb cb, void *cbdata);

extern void
spiflash_program_page(uint32_t addr, const uint8_t *src, uint8_t len, spi_cb cb, void *cbdata);

extern void
spiflash_read_page(uint8_t *dest, uint32_t addr, uint32_t len, spi_cb cb, void *cbdata);

extern void
spiflash_erase_sector(uint32_t addr, spi_cb cb, void *cbdata);

