#include <mchck.h>

enum spiflash_command {
        /* NO_OP is not technically a command but used to indicate no prelude */
        NO_OP = 0x0,
        WRITE_STATUS_REGISTER_1 = 0x1,
        PAGE_PROGRAM = 0x02,
        READ_DATA = 0x03,
        READ_STATUS_REGISTER_1 = 0x05,
        WRITE_ENABLE = 0x06,
        SECTOR_ERASE = 0x20,
        ENABLE_WRITE_STATUS_REGISTER = 0x50,
        BLOCK_ERASE_32KB = 0x52,
        BLOCK_ERASE_64KB = 0xD8,
        DEVICE_ERASE = 0xC7,
        GET_IDENTIFICATION = 0x9F,
};

struct spiflash_device onboard_flash = {
        .cs = SPI_PCS4
};

static void spiflash_schedule(struct spiflash_device *dev);

void
spiflash_pins_init(void)
{
        pin_mode(PIN_PTC0, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTC5, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTC6, PIN_MODE_MUX_ALT2);
        pin_mode(PIN_PTC7, PIN_MODE_MUX_ALT2);
}

static void
spiflash_queue_transaction(struct spiflash_device *dev,
                           struct spiflash_transaction *trans,
                           enum spiflash_command prelude,
                           bool wait_busy,
                           spiflash_transaction_done_cb *done_cb)
{
        trans->next = NULL;
        trans->dev = dev;
        trans->done_cb = done_cb;
        trans->prelude_command = prelude;
        trans->flags.wait_busy = wait_busy;
        trans->flags.queued = true;
        crit_enter();
        if (dev->queue) {
                struct spiflash_transaction *tail = dev->queue;
                while (tail->next != NULL)
                        tail = tail->next;
                tail->next = trans;
        } else
                dev->queue = trans;
        crit_exit();

        spiflash_schedule(dev);
}

static void
spiflash_transaction_done(struct spiflash_transaction *trans)
{
        struct spiflash_device *dev = trans->dev;
        dev->queue = trans->next;
        trans->flags.queued = false;
        trans->flags.running = false;
        trans->done_cb(trans);
        spiflash_schedule(dev);
}

static void
spiflash_check_write_completed_spi_cb(void *cbdata)
{
        struct spiflash_transaction *trans = cbdata;
        /* if not busy, call the user callback; otherwise, reissue get status */
        if (!(trans->spi_response[1] & 1)) {
                spiflash_transaction_done(trans);
        } else {
                spi_queue_xfer(&trans->dev->flash_spi_ctx, trans->dev->cs,
                               trans->spi_query, 1, trans->spi_response, 2,
                               spiflash_check_write_completed_spi_cb, trans);
        }
}

static void
spiflash_transaction_dispatched_spi_cb(void *cbdata)
{
        struct spiflash_transaction *trans = cbdata;
        if (trans->flags.wait_busy) {
                /* command and data transferred, wait for the busy flag to clear */
                trans->spi_query[0] = 0x05;
                spi_queue_xfer(&trans->dev->flash_spi_ctx, trans->dev->cs,
                               trans->spi_query, 1, trans->spi_response, 2,
                               spiflash_check_write_completed_spi_cb, trans);
        } else {
                spiflash_transaction_done(trans);
        }
}

static void
spiflash_run_transaction(struct spiflash_transaction *trans)
{
        spi_queue_xfer_sg(&trans->dev->flash_spi_ctx.ctx, trans->dev->cs,
                          trans->flash_tx_sg, trans->flash_rx_sg,
                          spiflash_transaction_dispatched_spi_cb, trans);
}

static void
spiflash_write_enabled_spi_cb(void *cbdata)
{
        spiflash_run_transaction(cbdata);
}

static void
spiflash_schedule(struct spiflash_device *dev)
{
        if (dev->queue == NULL)
                return;
        if (dev->queue->flags.running)
                return;

        struct spiflash_transaction *trans = dev->queue;
        trans->flags.running = true;
        if (trans->prelude_command != NO_OP) {
                /* send prelude, e.g. write enable, then run transaction */
                spi_queue_xfer(&trans->dev->flash_spi_ctx, trans->dev->cs,
                               &trans->prelude_command, 1, NULL, 0,
                               spiflash_write_enabled_spi_cb, trans);
        } else {
                spiflash_run_transaction(trans);
        }
}

static void
spiflash_setup_xfer(struct spiflash_transaction *trans,
                    uint16_t txlen, uint16_t rxlen)
{
        sg_init(trans->flash_tx_sg, trans->spi_query, txlen);
        sg_init(trans->flash_rx_sg, trans->spi_response, rxlen);
}

static void
spiflash_get_id_done_cb(struct spiflash_transaction *trans)
{
        trans->info_cb(trans->cbdata, trans->spi_response[1],
                       trans->spi_response[2], trans->spi_response[3]);
}

int
spiflash_get_id(struct spiflash_device *dev, struct spiflash_transaction *trans,
                spiflash_info_cb cb, void *cbdata)
{
        trans->spi_query[0] = GET_IDENTIFICATION;
        trans->spi_query[1] = trans->spi_query[2] = trans->spi_query[3] = 0x00;
        spiflash_setup_xfer(trans, 4, 4);

        trans->info_cb = cb;
        trans->cbdata = cbdata;
        spiflash_queue_transaction(dev, trans, NO_OP, false, spiflash_get_id_done_cb);
        return 0;
}

static void
spiflash_status_done_cb(struct spiflash_transaction *trans)
{
        trans->status_cb(trans->cbdata, trans->spi_response[1]);
}

int
spiflash_get_status(struct spiflash_device *dev, struct spiflash_transaction *trans,
                    spiflash_status_cb cb, void *cbdata)
{
        trans->spi_query[0] = READ_STATUS_REGISTER_1;
        spiflash_setup_xfer(trans, 1, 2);

        trans->status_cb = cb;
        trans->cbdata = cbdata;
        spiflash_queue_transaction(dev, trans, NO_OP, false, spiflash_status_done_cb);
        return 0;
}

static void
spiflash_spi_done_cb(struct spiflash_transaction *trans)
{
        if (trans->spi_cb)
                trans->spi_cb(trans->cbdata);
}

int
spiflash_read_page(struct spiflash_device *dev, struct spiflash_transaction *trans,
                   uint8_t *dest, uint32_t addr, uint32_t len, spi_cb cb, void *cbdata)
{
        trans->spi_query[0] = READ_DATA;
        trans->spi_query[1] = addr >> 16;
        trans->spi_query[2] = addr >> 8;
        trans->spi_query[3] = addr;

        trans->spi_cb = cb;
        trans->cbdata = cbdata;
        sg_init_list(trans->flash_tx_sg, 1, trans->spi_query, 4);
        sg_init_list(trans->flash_rx_sg, 2, trans->spi_response, 4, dest, len);
        spiflash_queue_transaction(dev, trans, NO_OP, false, spiflash_spi_done_cb);
        return 0;
}

static int
spiflash_write_cmd(struct spiflash_device *dev, struct spiflash_transaction *trans,
                   uint8_t cmd, uint32_t addr, const uint8_t *src, uint8_t len,
                   spi_cb cb, void *cbdata)
{
        trans->spi_query[0] = cmd;
        trans->spi_query[1] = addr >> 16;
        trans->spi_query[2] = addr >> 8;
        trans->spi_query[3] = addr;
        if (src)
                sg_init_list(trans->flash_tx_sg, 2, trans->spi_query, 4, src, len);
        else
                sg_init_list(trans->flash_tx_sg, 1, trans->spi_query, 4);
        sg_init_list(trans->flash_rx_sg, 0);

        trans->spi_cb = cb;
        trans->cbdata = cbdata;
        spiflash_queue_transaction(dev, trans, WRITE_ENABLE, cb != NULL ? true : false, spiflash_spi_done_cb);

        return 0;
}

int
spiflash_program_page(struct spiflash_device *dev, struct spiflash_transaction *trans,
                      uint32_t addr, const uint8_t *src, uint8_t len,
                      spi_cb cb, void *cbdata)
{
        return spiflash_write_cmd(dev, trans, PAGE_PROGRAM, addr, src, len, cb, cbdata);
}

int
spiflash_erase_sector(struct spiflash_device *dev, struct spiflash_transaction *trans,
                      uint32_t addr, spi_cb cb, void *cbdata)
{
        return spiflash_write_cmd(dev, trans, SECTOR_ERASE, addr, NULL, 0, cb, cbdata);
}

int
spiflash_erase_block(struct spiflash_device *dev, struct spiflash_transaction *trans,
                     uint32_t addr, int is_64KB, spi_cb cb, void *cbdata)
{
        return spiflash_write_cmd(dev, trans,
                                  is_64KB ? BLOCK_ERASE_64KB : BLOCK_ERASE_32KB,
                                  addr, NULL, 0, cb, cbdata);
}

int
spiflash_erase_device(struct spiflash_device *dev, struct spiflash_transaction *trans, spi_cb cb, void *cbdata)
{
        trans->spi_query[0] = DEVICE_ERASE;
        spiflash_setup_xfer(trans, 1, 0);
        trans->spi_cb = cb;
        trans->cbdata = cbdata;
        spiflash_queue_transaction(dev, trans, WRITE_ENABLE, true, spiflash_spi_done_cb);
        return 0;
}

int
spiflash_set_protection(struct spiflash_device *dev, struct spiflash_transaction *trans,
                        bool protected, spi_cb cb, void *cbdata)
{
        trans->spi_query[0] = WRITE_STATUS_REGISTER_1;
        trans->spi_query[1] = protected ? 0x3c : 0x00;
        spiflash_setup_xfer(trans, 2, 0);
        trans->spi_cb = cb;
        trans->cbdata = cbdata;
        spiflash_queue_transaction(dev, trans, ENABLE_WRITE_STATUS_REGISTER,
                                   false, spiflash_spi_done_cb);
        return 0;
}
