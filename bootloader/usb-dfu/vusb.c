typedef int usb_xfer_info_t;    /* XXX */

void
usb_enable(void)
{
}

static void *
usb_get_xfer_data(usb_xfer_info_t i)
{
}

static enum usb_tok_pid
usb_get_xfer_pid(usb_xfer_info_t i)
{
}

static void
usb_enable_xfers(void)
{
}

static void
usb_set_addr(int addr)
{
}

void
usb_ep_stall(int ep)
{
}

static void
usb_clear_transfers(void)
{
}

static inline size_t
usb_ep_get_transfer_size(int ep, enum usb_ep_dir dir, enum usb_ep_pingpong pingpong)
{
}

static void
usb_tx_queue_next(struct usbd_ep_pipe_state_t *s)
{
}

static void
usb_rx_queue_next(struct usbd_ep_pipe_state_t *s)
{
}
