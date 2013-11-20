#include <mchck.h>

static struct spi_ctx flash_spi_ctx;
static struct sg flash_tx_sg[2], flash_rx_sg[2];
static spi_cb *post_status_cb;
static union {
        spiflash_info_cb *info_cb;
        spiflash_status_cb *status_cb;
} current_cb;
static uint8_t spi_query[5];
static uint8_t spi_response[4];

void
spiflash_pins_init(void)
{
        pin_mode(PIN_PTC0, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTC5, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTC6, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTC7, PIN_MODE_MUX_ALT2);
}

static void
spiflash_get_id_cb(void *cbdata)
{
        current_cb.info_cb(cbdata, spi_response[1], spi_response[2], spi_response[3]);
}

int
spiflash_get_id(spiflash_info_cb cb, void *cbdata)
{
        if (spi_is_xfer_active())
                return -1;

        spi_query[0] = 0x9F;
        spi_query[1] = spi_query[2] = spi_query[3] = 0x00;

        current_cb.info_cb = cb;
        spi_queue_xfer(&flash_spi_ctx, SPI_PCS4, (uint8_t *)spi_query, 4, (uint8_t *)spi_response, 4, spiflash_get_id_cb, cbdata);
        return 0;
}

static void
spiflash_status_spi_cb(void *cbdata)
{
        current_cb.status_cb(cbdata, spi_response[1]);
}

int
spiflash_get_status(spiflash_status_cb cb, void *cbdata)
{
        if (spi_is_xfer_active())
                return -1;

        spi_query[0] = 0x05;
        current_cb.status_cb = cb;
        spi_queue_xfer(&flash_spi_ctx, SPI_PCS4, spi_query, 1, (uint8_t *)spi_response, 2, spiflash_status_spi_cb, cbdata);
        return 0;
}

static void
spiflash_erase_status_spi_cb(void *cbdata, uint8_t status)
{
        if (!(status & 1))
                post_status_cb(cbdata);
        else
                spiflash_get_status(spiflash_erase_status_spi_cb, cbdata);
}

static void
spiflash_erase_spi_cb(void *cbdata)
{
        spiflash_get_status(spiflash_erase_status_spi_cb, cbdata);
}

static void
spiflash_program_spi_cb(void *cbdata)
{
        /* Do the actual programming, wait for status to become non-busy */
        spi_queue_xfer_sg(&flash_spi_ctx.ctx, SPI_PCS4, flash_tx_sg, flash_rx_sg, spiflash_erase_spi_cb, cbdata);
}

int
spiflash_program_page(uint32_t addr, const uint8_t *src, uint8_t len, spi_cb cb, void *cbdata)
{
        if (spi_is_xfer_active())
                return -1;

        spi_query[0] = 0x02; /* second command (program) */
        spi_query[1] = addr >> 16;
        spi_query[2] = addr >> 8;
        spi_query[3] = addr;
        spi_query[4] = 0x06; /* first command (write enable) */
        sg_init_list(flash_tx_sg, 2, spi_query, 4, src, len);
        sg_init_list(flash_rx_sg, 0);
        post_status_cb = cb;
        /* write enable, then proceed with programming from the callback */
        spi_queue_xfer(&flash_spi_ctx, SPI_PCS4, spi_query + 4, 1, NULL, 0, spiflash_program_spi_cb, cbdata);
        
        return 0;
}

int
spiflash_read_page(uint8_t *dest, uint32_t addr, uint32_t len, spi_cb cb, void *cbdata)
{
        if (spi_is_xfer_active())
                return -1;

        spi_query[0] = 0x03;
        spi_query[1] = addr >> 16;
        spi_query[2] = addr >> 8;
        spi_query[3] = addr;
        sg_init_list(flash_tx_sg, 1, spi_query, 4);
        sg_init_list(flash_rx_sg, 2, spi_response, 4, dest, len);
        spi_queue_xfer_sg(&flash_spi_ctx.ctx, SPI_PCS4, flash_tx_sg, flash_rx_sg, cb, cbdata);

        return 0;
}

int
spiflash_erase_sector(uint32_t addr, spi_cb cb, void *cbdata)
{
        if (spi_is_xfer_active())
                return -1;

        spi_query[0] = 0x20; /* second command (erase) */
        spi_query[1] = addr >> 16;
        spi_query[2] = addr >> 8;
        spi_query[3] = addr;
        spi_query[4] = 0x06; /* first command (write enable) */
        post_status_cb = cb;
        sg_init_list(flash_tx_sg, 1, spi_query, 4);
        sg_init_list(flash_rx_sg, 0);
        /* write enable, then proceed with erasing from the callback */
        spi_queue_xfer(&flash_spi_ctx, SPI_PCS4, spi_query + 4, 1, NULL, 0, spiflash_program_spi_cb, cbdata);

        return 0;
}
