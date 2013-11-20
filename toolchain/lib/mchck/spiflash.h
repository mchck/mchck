#define SPIFLASH_MFGID_WINBOND 0x9F
#define SPIFLASH_MEMTYPE_WINBOND_FLASH 0x40
#define SPIFLASH_WINBOND_SIZE_1MB 0x14

typedef void (spiflash_info_cb)(void *cbdata, uint8_t mfg_id, uint8_t memtype, uint8_t capacity);
typedef void (spiflash_status_cb)(void *cbdata, uint8_t status);

struct spiflash_ctx {
        struct spi_ctx  flash_spi_ctx;
        struct sg       flash_tx_sg[2];
        struct sg       flash_rx_sg[2];
        spi_cb         *write_completed_cb;
        union {
                spiflash_info_cb        *info_cb;
                spiflash_status_cb      *status_cb;
        };
        void           *cbdata;
        uint8_t         spi_query[5];
        uint8_t         spi_response[4];
};

extern void
spiflash_pins_init(void);

extern int
spiflash_get_id(struct spiflash_ctx *ctx, spiflash_info_cb cb, void *cbdata);

extern int
spiflash_get_status(struct spiflash_ctx *ctx, spiflash_status_cb cb, void *cbdata);

extern int
spiflash_program_page(struct spiflash_ctx *ctx, uint32_t addr, const uint8_t *src, uint8_t len, spi_cb cb, void *cbdata);

extern int
spiflash_read_page(struct spiflash_ctx *ctx, uint8_t *dest, uint32_t addr, uint32_t len, spi_cb cb, void *cbdata);

extern int
spiflash_erase_sector(struct spiflash_ctx *ctx, uint32_t addr, spi_cb cb, void *cbdata);

extern int
spiflash_erase_block(struct spiflash_ctx *ctx, uint32_t addr, int is_64KB, spi_cb cb, void *cbdata);
