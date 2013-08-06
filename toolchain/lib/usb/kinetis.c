#include <mchck.h>

#define usb_xfer_info USB_STAT_t

#include <usb/usb.h>
#include "usb-internal.h"

/**
 * Kinetis USB driver notes:
 * We need to manually maintain the DATA0/1 toggling for the SIE.
 * SETUP transactions always start with a DATA0.
 *
 * The SIE internally uses pingpong (double) buffering, which is
 * easily confused with DATA0/DATA1 toggling, and I think even the
 * Freescale docs confuse the two notions.  When BD->DTS is set,
 * BD->DATA01 will be used to verify/discard incoming DATAx and it
 * will set the DATAx PID for outgoing tokens.  This is not described
 * as such in the Freescale Kinetis docs, but the Microchip PIC32 OTG
 * docs are more clear on this;  it seems that both Freescale and
 * Microchip use different versions of the same USB OTG IP core.
 *
 * http://ww1.microchip.com/downloads/en/DeviceDoc/61126F.pdf
 *
 * Clear CTL->TOKEN_BUSY after SETUP tokens.
 */


static struct USB_BD_t bdt[USB_MAX_EP * 2 *2] __attribute__((section(".usb_bdt")));

static struct USB_BD_t *
usb_get_bd(struct usbd_ep_pipe_state_t *s)
{
        return (&bdt[(s->ep_num << 2) | (s->ep_dir << 1) | s->pingpong]);
}

static struct USB_BD_t *
usb_get_bd_stat(struct USB_STAT_t *stat)
{
        return (((void *)(uintptr_t)bdt + (stat->raw << 1)));
}

void *
usb_get_xfer_data(struct usb_xfer_info *i)
{
        return (usb_get_bd_stat(i)->addr);
}

enum usb_tok_pid
usb_get_xfer_pid(struct usb_xfer_info *i)
{
        return (usb_get_bd_stat(i)->tok_pid);
}

int
usb_get_xfer_ep(struct usb_xfer_info *i)
{
        return (i->ep);
}

enum usb_ep_dir
usb_get_xfer_dir(struct usb_xfer_info *i)
{
        return (i->dir);
}

void
usb_enable_xfers(void)
{
        USB0.ctl.raw = ((struct USB_CTL_t){
                        .txd_suspend = 0,
                                .usben = 1
                                }).raw;
}

void
usb_set_addr(int addr)
{
        USB0.addr.raw = addr;
}


void
usb_pipe_stall(struct usbd_ep_pipe_state_t *s)
{
        volatile struct USB_BD_t *bd = usb_get_bd(s);
        bd->raw = ((struct USB_BD_BITS_t){
                        .stall = 1,
                                .own = 1
                                }).raw;
}

void
usb_pipe_unstall(struct usbd_ep_pipe_state_t *s)
{
        volatile struct USB_BD_t *bd = usb_get_bd(s);
        struct USB_BD_BITS_t state = { .raw = bd->raw };

        if (state.own && state.stall)
                bd->raw = 0;
}

void
usb_pipe_enable(struct usbd_ep_pipe_state_t *s)
{
        USB0.endpt[s->ep_num].raw |= ((struct USB_ENDPT_t){
                        .eptxen = s->ep_dir == USB_EP_TX,
                                .eprxen = s->ep_dir == USB_EP_RX,
                                .ephshk = 1, /* XXX ISO */
                                .epctldis = s->ep_num != 0
                                }).raw;
}

void
usb_pipe_disable(struct usbd_ep_pipe_state_t *s)
{
        USB0.endpt[s->ep_num].raw &= ~((struct USB_ENDPT_t){
                        .eptxen = s->ep_dir == USB_EP_TX,
                                .eprxen = s->ep_dir == USB_EP_RX,
                                .ephshk = 1,
                                .epctldis = 1
                                }).raw;
}

/* XXX what is this supposed to do? */
void
usb_clear_transfers(void)
{
}

size_t
usb_ep_get_transfer_size(struct usbd_ep_pipe_state_t *s)
{
        struct USB_BD_t *bd = usb_get_bd(s);
        return (bd->bc);
}

void
usb_queue_next(struct usbd_ep_pipe_state_t *s, void *addr, size_t len)
{
        volatile struct USB_BD_t *bd = usb_get_bd(s);

        bd->addr = addr;
        /* damn you bitfield problems */
        bd->raw = ((struct USB_BD_BITS_t){
                        .dts = 1,
                                .own = 1,
                                .data01 = s->data01,
                                .bc = len,
                                }).raw;
}

static void
usb_reset(void)
{
        /* reset pingpong state */
        /* For some obscure reason, we need to use or here. */
        USB0.ctl.raw |= ((struct USB_CTL_t){
                        .txd_suspend = 1,
                                .oddrst = 1,
                                }).raw;

        /* clear all interrupt bits - not sure if needed */
        USB0.istat.raw = 0xff;
        USB0.errstat.raw = 0xff;
        USB0.otgistat.raw = 0xff;

        /* zap also BDT pingpong & queued transactions */
        memset(bdt, 0, sizeof(bdt));
        USB0.addr.raw = 0;

        usb_restart();

        USB0.ctl.raw = ((struct USB_CTL_t){
                        .txd_suspend = 0,
                                .usben = 1
                                }).raw;
}

void
usb_enable(void)
{
        SIM.sopt2.usbsrc = 1;   /* usb from mcg */
        SIM.scgc4.usbotg = 1;   /* enable usb clock */

        /* reset module - not sure if needed */
        USB0.usbtrc0.raw = ((struct USB_USBTRC0_t){
                        .usbreset = 1
                                }).raw;
        while (USB0.usbtrc0.usbreset)
                /* NOTHING */;

        USB0.bdtpage1 = (uintptr_t)bdt >> 8;
        USB0.bdtpage2 = (uintptr_t)bdt >> 16;
        USB0.bdtpage3 = (uintptr_t)bdt >> 24;

        USB0.control.raw = ((struct USB_CONTROL_t){
                        .dppullupnonotg = 1 /* enable pullup */
                                }).raw;

        USB0.usbctrl.raw = 0; /* resume peripheral & disable pulldowns */
        usb_reset();          /* this will start usb processing */

        /* we're only interested in reset and transfers */
        USB0.inten.raw = ((struct USB_ISTAT_t){
                        .tokdne = 1,
                                .usbrst = 1,
                                .stall = 1
                                }).raw;

        int_enable(IRQ_USB0);
}

void
USB0_Handler(void)
{
        struct USB_ISTAT_t stat = USB0.istat;
        if (stat.usbrst) {
                usb_reset();
                return;
        }
        if (stat.stall) {
                /* XXX need more work for non-0 ep */
                volatile struct USB_BD_t *bd = usb_get_bd(&usb.ep_state[0].rx);
                if (bd->stall)
                        usb_setup_control();
        }
        if (stat.tokdne) {
                struct usb_xfer_info stat = USB0.stat;
                usb_handle_transaction(&stat);
        }
        USB0.istat.raw = stat.raw;
}
