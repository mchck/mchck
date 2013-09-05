#include <mchck.h>

static struct spi_ctx *spi_ctx;

static void
spi_start_xfer(void)
{
        SPI0.mcr.raw = ((struct SPI_MCR){
                        .mstr = 1,
                                .dconf = SPI_DCONF_SPI,
                                .rooe = 1,
                                .pcsis = 0b11111,
                                .clr_txf = 1,
                                .clr_rxf = 1,
                                .halt = 0
                                }).raw;
}

void
spi_xfer(enum spi_pcs pcs,
         const uint8_t *tx_buf, size_t tx_len,
         uint8_t *rx_buf, size_t rx_len,
         struct spi_ctx *ctx,
         spi_cb *cb, void *cbdata)
{
        *ctx = (struct spi_ctx){
                .tx_pos = 0,
                .rx_pos = 0,
                .pcs = pcs,
                .tx_buf = tx_buf,
                .tx_len = tx_len,
                .rx_buf = rx_buf,
                .rx_len = rx_len,
                .cb = cb,
                .cbdata = cbdata,
                .next = NULL
        };

        crit_enter();
        /* search for tail and append */
        for (struct spi_ctx **c = &spi_ctx; ; c = &(*c)->next) {
                if (*c == NULL) {
                        *c = ctx;
                        /* we're at the head, so start xfer */
                        if (*c == spi_ctx)
                                spi_start_xfer();
                        break;
                }
        }
        crit_exit();
}

void
SPI0_Handler(void)
{
        if (spi_ctx == NULL)
                return;

        size_t xfer_len = spi_ctx->rx_len > spi_ctx->tx_len ? spi_ctx->rx_len : spi_ctx->tx_len;
        int rx_left = spi_ctx->rx_len - spi_ctx->rx_pos;
        int tx_left = xfer_len - spi_ctx->tx_pos;

        for (;;) {
                struct SPI_SR status;

                status.raw = SPI0.sr.raw;
                if (!((status.rfdf && rx_left > 0) || (status.tfff && tx_left > 0)))
                        break;

                while (rx_left > 0 && SPI0.sr.rfdf) {
                        spi_ctx->rx_buf[spi_ctx->rx_pos++] = SPI0.popr;
                        --rx_left;
                        /* not sure if this is needed - will it autoclear? */
                        SPI0.sr.raw = ((struct SPI_SR){.rfdf = 1}).raw;
                }

                while (tx_left > 0 && SPI0.sr.tfff) {
                        int last = spi_ctx->tx_pos + 1 == xfer_len;
                        uint8_t data;

                        if (spi_ctx->tx_pos < spi_ctx->tx_len)
                                data = spi_ctx->tx_buf[spi_ctx->tx_pos++];
                        else
                                data = 0xff;

                        SPI0.pushr.raw = ((struct SPI_PUSHR){
                                        .cont = !last,
                                                .ctas = 0,
                                                .pcs = spi_ctx->pcs,
                                                .txdata = data
                                                }).raw;
                        --tx_left;
                        SPI0.sr.raw = ((struct SPI_SR){.tfff = 1}).raw;
                }
        }

        if (rx_left <= 0 && tx_left <= 0) {
                struct spi_ctx *ctx = spi_ctx;

                spi_ctx = ctx->next;
                if (spi_ctx != NULL)
                        spi_start_xfer();
                if (ctx->cb != NULL)
                        ctx->cb(ctx->rx_buf, ctx->rx_len, ctx->cbdata);
        }
}

void
spi_init(void)
{
        SIM.scgc6.spi0 = 1; /* enable SPI clock */
        SPI0.ctar[0].raw = ((struct SPI_CTAR){
                        .fmsz = 7,
                                .cpol = 0,
                                .cpha = 0,
                                .cssck = 0b1000,
                                .asc = 0b1000,
                                .dt = 0b1000,
                                .br = 0b1000
                                }).raw;
        SPI0.rser.raw = ((struct SPI_RSER){
                        .tfff_re = 1,
                                .rfdf_re = 1,
                                }).raw;
#ifndef SHORT_ISR
        int_enable(IRQ_SPI0);
#endif
}
