#define usb_xfer_info USB_STAT_t

#include "usb.h"
#include "usb-internal.h"

#include "usbotg.h"
#include "sim.h"
#include "mcg.h"


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
        /* SIM.scgc4.usbotg = 1;    /\* enable usb clock *\/ */

        /* USB0_USBCTRL = 0; */

        /* USB0.usbtrc0.usbreset = 1; */
        /* while (USB0.usbtrc0.usbreset == 1) */
        /*         /\* NOTHING *\/; */

        /* USB0_USBTRC0 |= 0x40; */

        /* USB0.bdtpage1 = (uintptr_t)bdt >> 8; */
        /* USB0.bdtpage2 = (uintptr_t)bdt >> 16; */
        /* USB0.bdtpage3 = (uintptr_t)bdt >> 24; */

        /* USB0.control.dppullupnonotg = 1; */

        /* USB0.ctl.usben = 1; */

        /* USB0.usbctrl.susp = 0;   /\* resume peripheral *\/ */

        /* USB0.inten.tokdne = 1; */
        /* USB0.inten.usbrst = 1; */

        /* /\* USB0.otgctl.dphigh = 1; *\/ */
        /* /\* USB0.otgctl.otgen = 1; *\/ */
        /* USB0_OTGCTL = USB_OTGCTL_DPHIGH_MASK | USB_OTGCTL_OTGEN_MASK; */

        /* USB0.endpt[0].ephshk = 1; */
        /* USB0.endpt[0].eprxen = 1; */
        /* USB0.endpt[0].eptxen = 1; */

	// this basically follows the flowchart in the Kinetis
	// Quick Reference User Guide, Rev. 1, 03/2012, page 141

	// assume 48 MHz clock already running
	// SIM - enable clock
	SIM_SCGC4 |= SIM_SCGC4_USBOTG_MASK;

	// reset USB module
	USB0_USBTRC0 = USB_USBTRC0_USBRESET_MASK;
	while ((USB0_USBTRC0 & USB_USBTRC0_USBRESET_MASK) != 0) ; // wait for reset to end

	// set desc table base addr
	USB0_BDTPAGE1 = ((uint32_t)bdt) >> 8;
	USB0_BDTPAGE2 = ((uint32_t)bdt) >> 16;
	USB0_BDTPAGE3 = ((uint32_t)bdt) >> 24;

	// clear all ISR flags
	USB0_ISTAT = 0xFF;
	USB0_ERRSTAT = 0xFF;
	USB0_OTGISTAT = 0xFF;

	USB0_USBTRC0 |= 0x40; // undocumented bit

	// enable USB
	USB0_CTL = USB_CTL_USBENSOFEN_MASK;
	USB0_USBCTRL = 0;

	// enable reset interrupt
	USB0_INTEN = USB_INTEN_USBRSTEN_MASK | USB_INTEN_TOKDNEEN_MASK;

	/* // enable interrupt in NVIC... */
	/* NVIC_ENABLE_IRQ(IRQ_USBOTG); */

	// enable d+ pullup
	USB0_CONTROL = USB_CONTROL_DPPULLUPNONOTG_MASK;
}

#define FSET(field, type, ...)                        \
        (field = ({ type __field = __VA_ARGS__; __field; }))

void
usb_intr(void)
{
        while (USB0.istat.usbrst) {
                USB0_ERRSTAT = 0xFF;
                USB0_ISTAT = 0xFF;
                USB0.ctl.oddrst = 1;
                USB0.addr.addr = 0;
                memset(bdt, 0, sizeof(bdt));
                USB0.endpt[0].eptxen = 1;
                USB0.endpt[0].eprxen = 1;
                USB0.endpt[0].ephshk = 1;
                USB0.ctl.oddrst = 0;
                usb_restart();
        }
        while (USB0.istat.tokdne) {
                struct usb_xfer_info stat = USB0.stat;
                FSET(USB0.istat, struct USB_ISTAT_t, { .tokdne = 1 });
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
        return (((void *)(uintptr_t)bdt + (stat->stat << 1)));
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
        struct USB_BD_t *bd = usb_get_bd(0, dir, s->pingpong);

        bd->addr = addr;
        /* damn you bitfield problems */
        bd->bd = (len << 16) | (1 << 3) | (s->data01 << 6) | (1 << 7);
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
