#include <mchck.h>

static struct spi_ctx_bare *spi_ctx;

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
        SPI0.rser.raw = ((struct SPI_RSER){
                        .tfff_re = 1,
                                .rfdf_re = spi_ctx->rx != NULL,
                                .eoqf_re = 1,
                                }).raw;
        SPI0.sr.raw |= 0;
}

static void
spi_stop_xfer(void)
{
        SPI0.mcr.raw = ((struct SPI_MCR){
                        .mstr = 1,
                                .dconf = SPI_DCONF_SPI,
                                .rooe = 1,
                                .pcsis = 0b11111,
                                .clr_txf = 1,
                                .clr_rxf = 1,
                                .halt = 1
                                }).raw;
        SPI0.rser.raw = 0;
        SPI0.sr.raw |= 0;
}

bool spi_is_idle(void)
{
        return spi_ctx == NULL;
}

int
spi_is_xfer_active(void)
{
        return !SPI0.mcr.halt;
}

int
spi_queue_xfer(struct spi_ctx *ctx,
               enum spi_pcs pcs,
               const uint8_t *txbuf, uint16_t txlen,
               uint8_t *rxbuf, uint16_t rxlen,
               spi_cb *cb, void *cbdata)
{
        return spi_queue_xfer_sg(&ctx->ctx, pcs,
                                 sg_init(&ctx->tx_sg, (void *)txbuf, txlen),
                                 sg_init(&ctx->rx_sg, rxbuf, rxlen),
                                 cb, cbdata);
}

int
spi_queue_xfer_sg(struct spi_ctx_bare *ctx,
                  enum spi_pcs pcs,
                  struct sg *tx, struct sg *rx,
                  spi_cb *cb, void *cbdata)
{
        if (ctx->queued)
                return 1;

        *ctx = (struct spi_ctx_bare){
                .tx = sg_simplify(tx),
                .rx = sg_simplify(rx),
                .pcs = pcs,
                .cb = cb,
                .cbdata = cbdata,
                .queued = true,
                .next = NULL
        };

        size_t tx_len = sg_total_length(ctx->tx);
        size_t rx_len = sg_total_length(ctx->rx);
        if (rx_len > tx_len)
                ctx->rx_tail = rx_len - tx_len;

        crit_enter();
        /* search for tail and append */
        for (struct spi_ctx_bare **c = &spi_ctx; ; c = &(*c)->next) {
                if (*c == NULL) {
                        *c = ctx;
                        /* we're at the head, so start xfer */
                        if (c == &spi_ctx)
                                spi_start_xfer();
                        break;
                }
        }
        crit_exit();
        return 0;
}

void
SPI0_Handler(void)
{
again:
        if (spi_ctx == NULL)
                return;

        for (;;) {
                struct SPI_SR status, flags;

                status.raw = SPI0.sr.raw;
                flags.raw = status.raw & SPI0.rser.raw;

                if (flags.rfdf && spi_ctx->rx) {
                        for (int i = status.rxctr; i > 0 && spi_ctx->rx; --i, sg_move(&spi_ctx->rx, 1)) {
                                uint8_t d = SPI0.popr;
                                if (sg_data(spi_ctx->rx) != NULL)
                                        *sg_data(spi_ctx->rx) = d;
                        }
                        /* disable interrupt if we're done receiving */
                        if (!spi_ctx->rx)
                                SPI0.rser.rfdf_re = 0;
                        SPI0.sr.raw = ((struct SPI_SR){ .rfdf = 1 }).raw;
                } else if ((spi_ctx->tx || spi_ctx->rx_tail > 0) && flags.tfff) {
                        int more = 0;
                        uint8_t data;

                        if (spi_ctx->tx) {
                                data = *sg_data(spi_ctx->tx);
                                if (sg_move(&spi_ctx->tx, 1) != SG_END)
                                        more = 1;
                                else
                                        more = 0;
                        } else {
                                data = 0xff;
                                --spi_ctx->rx_tail;
                        }
                        more = more || (spi_ctx->rx_tail > 0);

                        SPI0.pushr.raw = ((struct SPI_PUSHR){
                                        .cont = more,
                                                .ctas = 0,
                                                .eoq = !more,
                                                .pcs = spi_ctx->pcs,
                                                .txdata = data
                                                }).raw;
                        SPI0.sr.raw = ((struct SPI_SR){ .tfff = 1 }).raw;
                } else if (flags.eoqf && !spi_ctx->tx && !spi_ctx->rx) {
                        /* transfer done */
                        struct spi_ctx_bare *ctx = spi_ctx;

                        crit_enter();
                        ctx->queued = false;
                        spi_ctx = ctx->next;
                        if (spi_ctx != NULL)
                                spi_start_xfer();
                        else
                                spi_stop_xfer();
                        crit_exit();
                        if (ctx->cb != NULL)
                                ctx->cb(ctx->cbdata);
                        goto again;
                } else {
                        break;
                }
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
        int_enable(IRQ_SPI0);
}
