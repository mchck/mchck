#include <mchck/sg.h>
#include <stdbool.h>

enum spi_pcs {
        SPI_PCS0 = 1 << 0,
        SPI_PCS1 = 1 << 1,
        SPI_PCS2 = 1 << 2,
        SPI_PCS3 = 1 << 3,
        SPI_PCS4 = 1 << 4,
        SPI_PCS5 = 1 << 5,
};

typedef void (spi_cb)(void *cbdata);


struct spi_ctx_bare {
        struct sg           *tx;
        struct sg           *rx;
        uint16_t             rx_tail;
        enum spi_pcs         pcs;
        spi_cb              *cb;
        void                *cbdata;
        struct spi_ctx_bare *next;
        bool                 queued;
};

struct spi_ctx {
        struct spi_ctx_bare ctx;
        struct sg tx_sg;
        struct sg rx_sg;
};

int spi_queue_xfer(struct spi_ctx *ctx,
                   enum spi_pcs pcs,
                   const uint8_t *txbuf, uint16_t txlen,
                   uint8_t *rxbuf, uint16_t rxlen,
                   spi_cb *cb, void *cbdata);
int spi_queue_xfer_sg(struct spi_ctx_bare *ctx,
                      enum spi_pcs pcs,
                      struct sg *tx, struct sg *rx,
                      spi_cb *cb, void *cbdata);
void spi_init(void);

bool spi_is_idle(void);

int spi_is_xfer_active(void);
