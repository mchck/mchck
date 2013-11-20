#include <mchck.h>

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
        struct spiflash_ctx *ctx = cbdata;
        ctx->info_cb(ctx->cbdata, ctx->spi_response[1], ctx->spi_response[2], ctx->spi_response[3]);
}

int
spiflash_get_id(struct spiflash_ctx *ctx, spiflash_info_cb cb, void *cbdata)
{
        ctx->spi_query[0] = 0x9F;
        ctx->spi_query[1] = ctx->spi_query[2] = ctx->spi_query[3] = 0x00;

        ctx->info_cb = cb;
        ctx->cbdata = cbdata;
        spi_queue_xfer(&ctx->flash_spi_ctx, SPI_PCS4, (uint8_t *)ctx->spi_query, 4, (uint8_t *)ctx->spi_response, 4, spiflash_get_id_cb, ctx);
        return 0;
}

static void
spiflash_status_spi_cb(void *cbdata)
{
        struct spiflash_ctx *ctx = cbdata;
        ctx->status_cb(ctx->cbdata, ctx->spi_response[1]);
}

int
spiflash_get_status(struct spiflash_ctx *ctx, spiflash_status_cb cb, void *cbdata)
{
        ctx->spi_query[0] = 0x05;
        ctx->status_cb = cb;
        ctx->cbdata = cbdata;
        spi_queue_xfer(&ctx->flash_spi_ctx, SPI_PCS4, ctx->spi_query, 1, ctx->spi_response, 2, spiflash_status_spi_cb, ctx);
        return 0;
}

static void
spiflash_check_write_completed_spi_cb(void *cbdata)
{
        struct spiflash_ctx *ctx = cbdata;
        /* if not busy, call the user callback; otherwise, reissue get status */
        if (!(ctx->spi_response[1] & 1))
                ctx->write_completed_cb(ctx->cbdata);
        else
                spi_queue_xfer(&ctx->flash_spi_ctx, SPI_PCS4, ctx->spi_query, 1, ctx->spi_response, 2, spiflash_check_write_completed_spi_cb, ctx);
}

static void
spiflash_write_dispatched_spi_cb(void *cbdata)
{
        /* command and data transferred, wait for the busy flag to clear */
        struct spiflash_ctx *ctx = cbdata;
        ctx->spi_query[0] = 0x05;
        spi_queue_xfer(&ctx->flash_spi_ctx, SPI_PCS4, ctx->spi_query, 1, ctx->spi_response, 2, spiflash_check_write_completed_spi_cb, ctx);
}

static void
spiflash_write_enabled_spi_cb(void *cbdata)
{
        struct spiflash_ctx *ctx = cbdata;
        /* Send the queued operation, then wait for the busy flag to clear */
        spi_queue_xfer_sg(&ctx->flash_spi_ctx.ctx, SPI_PCS4, ctx->flash_tx_sg, ctx->flash_rx_sg, spiflash_write_dispatched_spi_cb, ctx);
}

int
spiflash_read_page(struct spiflash_ctx *ctx, uint8_t *dest, uint32_t addr, uint32_t len, spi_cb cb, void *cbdata)
{
        ctx->spi_query[0] = 0x03;
        ctx->spi_query[1] = addr >> 16;
        ctx->spi_query[2] = addr >> 8;
        ctx->spi_query[3] = addr;
        ctx->cbdata = cbdata;
        sg_init_list(ctx->flash_tx_sg, 1, ctx->spi_query, 4);
        sg_init_list(ctx->flash_rx_sg, 2, ctx->spi_response, 4, dest, len);
        spi_queue_xfer_sg(&ctx->flash_spi_ctx.ctx, SPI_PCS4, ctx->flash_tx_sg, ctx->flash_rx_sg, cb, ctx);

        return 0;
}

static int
spiflash_write_cmd(struct spiflash_ctx *ctx, uint8_t cmd, uint32_t addr, const uint8_t *src, uint8_t len, spi_cb cb, void *cbdata)
{
        ctx->spi_query[0] = cmd; /* second command (program) */
        ctx->spi_query[1] = addr >> 16;
        ctx->spi_query[2] = addr >> 8;
        ctx->spi_query[3] = addr;
        ctx->spi_query[4] = 0x06; /* first command (write enable) */
        if (src)
                sg_init_list(ctx->flash_tx_sg, 2, ctx->spi_query, 4, src, len);
        else
                sg_init_list(ctx->flash_tx_sg, 1, ctx->spi_query, 4);
        sg_init_list(ctx->flash_rx_sg, 0);
        ctx->write_completed_cb = cb;
        ctx->cbdata = cbdata;
        /* write enable, then proceed with programming from the callback */
        spi_queue_xfer(&ctx->flash_spi_ctx, SPI_PCS4, ctx->spi_query + 4, 1, NULL, 0, spiflash_write_enabled_spi_cb, ctx);
        
        return 0;
}

int
spiflash_program_page(struct spiflash_ctx *ctx, uint32_t addr, const uint8_t *src, uint8_t len, spi_cb cb, void *cbdata)
{
        return spiflash_write_cmd(ctx, 0x02, addr, src, len, cb, cbdata);
}

int
spiflash_erase_sector(struct spiflash_ctx *ctx, uint32_t addr, spi_cb cb, void *cbdata)
{
        return spiflash_write_cmd(ctx, 0x20, addr, NULL, 0, cb, cbdata);
}

int
spiflash_erase_block(struct spiflash_ctx *ctx, uint32_t addr, int is_64KB, spi_cb cb, void *cbdata)
{
        return spiflash_write_cmd(ctx, is_64KB ? 0xD8 : 0x52, addr, NULL, 0, cb, cbdata);
}

