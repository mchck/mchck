#include <stdbool.h>

#define SPIFLASH_MFGID_WINBOND 0x9F
#define SPIFLASH_MEMTYPE_WINBOND_FLASH 0x40
#define SPIFLASH_WINBOND_SIZE_1MB 0x14

typedef void (spiflash_info_cb)(void *cbdata, uint8_t mfg_id, uint8_t memtype,
                                uint8_t capacity);
typedef void (spiflash_status_cb)(void *cbdata, uint8_t status);


struct spiflash_device;
struct spiflash_transaction;

typedef void (spiflash_transaction_done_cb)(struct spiflash_transaction *trans);

struct spiflash_transaction {
        struct spiflash_transaction_flags {
                UNION_STRUCT_START(8);
                unsigned wait_busy    : 1; // will set BUSY flag
                unsigned running      : 1; // currently in progress
                unsigned queued       : 1; // on the queue (either running or waiting to start)
                UNION_STRUCT_END;
        } flags;
        /* room for a single command sent before the actual command.
           Used to send WRITE_ENABLE, etc. */
        uint8_t         prelude_command;
        struct sg       flash_tx_sg[2];
        struct sg       flash_rx_sg[2];
        uint8_t         spi_query[5];
        uint8_t         spi_response[4];
        union {
                spiflash_info_cb        *info_cb;
                spiflash_status_cb      *status_cb;
                spi_cb                  *spi_cb;
        };
        void           *cbdata;

        spiflash_transaction_done_cb *done_cb;
        struct spiflash_device *dev;
        struct spiflash_transaction *next;
};

struct spiflash_device {
        struct spi_ctx  flash_spi_ctx;
        enum spi_pcs cs;
        struct spiflash_transaction *queue;
};

extern struct spiflash_device onboard_flash;

extern void
spiflash_pins_init(void);

extern int
spiflash_get_id(struct spiflash_device *dev, struct spiflash_transaction *trans,
                spiflash_info_cb cb, void *cbdata);

extern int
spiflash_get_status(struct spiflash_device *dev, struct spiflash_transaction *trans,
                    spiflash_status_cb cb, void *cbdata);

extern int
spiflash_program_page(struct spiflash_device *dev, struct spiflash_transaction *trans,
                      uint32_t addr, const uint8_t *src, uint8_t len, spi_cb cb, void *cbdata);

extern int
spiflash_read_page(struct spiflash_device *dev, struct spiflash_transaction *trans,
                   uint8_t *dest, uint32_t addr, uint32_t len, spi_cb cb, void *cbdata);

extern int
spiflash_erase_sector(struct spiflash_device *dev, struct spiflash_transaction *trans,
                      uint32_t addr, spi_cb cb, void *cbdata);

extern int
spiflash_erase_block(struct spiflash_device *dev, struct spiflash_transaction *trans,
                     uint32_t addr, int is_64KB, spi_cb cb, void *cbdata);

extern int
spiflash_erase_device(struct spiflash_device *dev, struct spiflash_transaction *trans,
                     spi_cb cb, void *cbdata);

extern int
spiflash_set_protection(struct spiflash_device *dev, struct spiflash_transaction *trans,
                        bool protected, spi_cb cb, void *cbdata);
