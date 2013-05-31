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


#ifndef USB_NUM_EP
#define USB_NUM_EP 1
#endif

static struct USB_BD_t bdt[USB_NUM_EP * 2 *2] __attribute__((section(".usb_bdt")));


void
usb_enable(void)
{
        SIM.scgc4.usbotg = 1;   /* enable usb clock */

        /* reset module - not sure if needed */
        USB0.usbtrc0.usbreset = 1;
        while (USB0.usbtrc0.usbreset == 1)
                /* NOTHING */;

        USB0.bdtpage1 = (uintptr_t)bdt >> 8;
        USB0.bdtpage2 = (uintptr_t)bdt >> 16;
        USB0.bdtpage3 = (uintptr_t)bdt >> 24;

        USB0.control.dppullupnonotg = 1; /* enable pullup */
        USB0.ctl.usben = 1;              /* enable SIE */
        USB0.usbctrl.susp = 0;           /* resume peripheral */

        /* clear all interrupt bits - not sure if needed */
	USB0.istat.raw = 0xff;
	USB0.errstat.raw = 0xff;
	USB0.otgistat.raw = 0xff;

        /* we're only interested in reset and transfers */
        USB0.inten.tokdne = 1;
        USB0.inten.usbrst = 1;
}

void
usb_intr(void)
{
        while (USB0.istat.usbrst) {
                /* clear interrupts */
                USB0.errstat.raw = 0xff;
                USB0.istat.raw = 0xff;
                /* reset pingpong state */
                USB0.ctl.oddrst = 1;
                /* zap also BDT pingpong & queued transactions */
                memset(bdt, 0, sizeof(bdt));
                USB0.endpt[0].eptxen = 1;
                USB0.endpt[0].eprxen = 1;
                USB0.endpt[0].ephshk = 1;
                /* stop resetting pingpong state - not sure if needed */
                USB0.ctl.oddrst = 0;
                USB0.addr.addr = 0;
                usb_restart();
        }
        while (USB0.istat.tokdne) {
                struct usb_xfer_info stat = USB0.stat;
                USB0.istat.raw = ((struct USB_ISTAT_t){ .tokdne = 1 }).raw;
                usb_handle_transaction(&stat);
        }
}

static struct USB_BD_t *
usb_get_bd(int ep, enum usb_ep_dir dir, enum usb_ep_pingpong pingpong)
{
        return (&bdt[(ep << 2) | (dir << 1) | pingpong]);
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

void
usb_enable_xfers(void)
{
        USB0.ctl.txd_suspend = 0;
}

void
usb_set_addr(int addr)
{
        USB0.addr.addr = addr;
}


/**
 * Stalls the EP.  SETUP transactions automatically unstall an EP.
 * XXX really?
 */
/* void */
/* usb_ep_stall(int ep) */
/* { */
/*         USB0.endpt[ep].epstall = 1; */
/* } */

/* Only EP0 for now; clears all pending transfers. XXX invoke callbacks? */
void
usb_clear_transfers(void)
{
        struct USB_BD_t *bd = bdt;

        memset(bd, 0, USB_NUM_EP * sizeof(*bd) * 4);
}

inline size_t
usb_ep_get_transfer_size(int ep, enum usb_ep_dir dir, enum usb_ep_pingpong pingpong)
{
        struct USB_BD_t *bd = usb_get_bd(ep, dir, pingpong);
        return (bd->bc);
}

void
usb_queue_next(struct usbd_ep_pipe_state_t *s, void *addr, size_t len, enum usb_ep_dir dir)
{
        volatile struct USB_BD_t *bd = usb_get_bd(0, dir, s->pingpong);

        bd->addr = addr;
        /* damn you bitfield problems */
        bd->raw = ((struct USB_BD_BITS_t){
                        .dts = 1,
                                .own = 1,
                                .data01 = s->data01,
                                .bc = len,
                                }).raw;
}

void
usb_tx_queue_next(struct usbd_ep_pipe_state_t *s, void *addr, size_t len)
{
        usb_queue_next(s, addr, len, USB_EP_TX);
}

void
usb_rx_queue_next(struct usbd_ep_pipe_state_t *s, void *addr, size_t len)
{
        usb_queue_next(s, addr, len, USB_EP_RX);
}
