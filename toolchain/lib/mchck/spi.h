enum spi_pcs {
        SPI_PCS0 = 1 << 0,
        SPI_PCS1 = 1 << 1,
        SPI_PCS2 = 1 << 2,
        SPI_PCS3 = 1 << 3,
        SPI_PCS4 = 1 << 4,
        SPI_PCS5 = 1 << 5,
};

typedef void (spi_cb)(uint8_t *buf, size_t len, void *cbdata);

struct spi_ctx {
        int             rx_pos;
        int             tx_pos;
        enum spi_pcs    pcs;
        const uint8_t  *tx_buf;
        size_t          tx_len;
        uint8_t        *rx_buf;
        size_t          rx_len;
        spi_cb         *cb;
        void           *cbdata;
        struct spi_ctx *next;
};

void spi_xfer(enum spi_pcs pcs,
              const uint8_t *tx_buf, size_t tx_len,
              uint8_t *rx_buf, size_t rx_len,
              struct spi_ctx *ctx,
              spi_cb *cb, void *cbdata);
void spi_init(void);
