#define usb_xfer_info USB_STAT_t

#include "usb.h"


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

/**
 * Hardware structures
 */

struct USB_BD_t {
        union /* bitfields */ {
                struct /* common */ {
                        uint32_t _rsvd0  : 6;
                        enum usb_data01 data01 : 1;
                        uint32_t own     : 1;
                        uint32_t _rsvd1  : 8;
                        uint32_t bc      : 10;
                        uint32_t _rsvd2  : 6;
                } __packed;
                struct /* USB-FS */ {
                        uint32_t _rsvd3  : 2;
                        uint32_t stall   : 1;
                        uint32_t dts     : 1;
                        uint32_t ninc    : 1;
                        uint32_t keep    : 1;
                        uint32_t _rsvd4  : 26;
                } __packed;
                struct /* processor */ {
                        uint32_t _rsvd5  : 2;
                        enum usb_tok_pid tok_pid : 4;
                        uint32_t _rsvd6  : 26;
                } __packed;
                uint32_t bd;
        }; /* union bitfields */
        void *addr;
};
CTASSERT_SIZE_BYTE(struct USB_BD_t, 8);

struct USB_STAT_t {
        union {
                struct {
                        uint8_t _rsvd0 : 2;
                        enum usb_ep_pingpong pingpong : 1;
                        enum usb_ep_dir dir : 1;
                        uint8_t ep : 4;
                };
                uint32_t stat;
        };
};
CTASSERT_SIZE_BIT(struct USB_STAT_t, 32);

struct USB_ENDPT_t {
        uint8_t ephshk : 1;
        uint8_t epstall : 1;
        uint8_t eptxen : 1;
        uint8_t eprxen : 1;
        uint8_t epctldis : 1;
        uint8_t _rsvd0 : 1;
        uint8_t retrydis : 1;
        uint8_t hostwohub : 1;
        uint32_t _rsvd1 : 24;
} __packed;
CTASSERT_SIZE_BIT(struct USB_ENDPT_t, 32);

struct USB_ADDR_t {
        uint8_t addr : 7;
        uint8_t lsen : 1;
} __packed;
CTASSERT_SIZE_BIT(struct USB_ADDR_t, 8);

struct USB_CTL_t {
        union {
                struct /* common */ {
                        uint8_t _rsvd1 : 1;
                        uint8_t oddrst : 1;
                        uint8_t resume : 1;
                        uint8_t _rsvd2 : 3;
                        uint8_t se0 : 1;
                        uint8_t jstate : 1;
                } __packed;
                struct /* host */ {
                        uint8_t sofen : 1;
                        uint8_t _rsvd3 : 2;
                        uint8_t hostmodeen : 1;
                        uint8_t reset : 1;
                        uint8_t token_busy : 1;
                        uint8_t _rsvd4 : 2;
                } __packed;
                struct /* device */ {
                        uint8_t usben : 1;
                        uint8_t _rsvd5 : 4;
                        uint8_t txd_suspend : 1;
                        uint8_t _rsvd6 : 2;
                } __packed;
        };
} __packed;
CTASSERT_SIZE_BIT(struct USB_CTL_t, 8);

extern volatile struct USB_ENDPT_t USB0_ENDPT[16];
extern volatile struct USB_ADDR_t USB0_ADDR;
extern volatile struct USB_CTL_t USB0_CTL;

#ifndef USB_NUM_EP
#define USB_NUM_EP 1
#endif

static struct USB_BD_t bdt[USB_NUM_EP * 2 *2];


void
usb_enable(void)
{
        /* clock distribution? */
        /* INTEN->(TOKDNE,USBRST)=1 */
        /* BDTPAGE1,2,3 */
        /* ENDPT0->(EPRXEN,EPTXEN,EPHSHK)=1 */
        /* USBCTRL->(SUSP,PDE)=0 */
        /* CTL->USBENSOFEN=1 */
}

void
usb_intr(void)
{
        /* check STAT->(ENDP,TX,ODD) */
        /* check ERRSTAT->(DMAERR) */
        /* read BDT entry */
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
        USB0_CTL.txd_suspend = 0;
}

void
usb_set_addr(int addr)
{
        USB0_ADDR.addr = addr;
}


/**
 * Stalls the EP.  SETUP transactions automatically unstall an EP.
 */
void
usb_ep_stall(int ep)
{
        USB0_ENDPT[ep].epstall = 1;
}

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
usb_tx_queue_next(struct usbd_ep_pipe_state_t *s, void *addr, size_t len)
{
        struct USB_BD_t *bd = usb_get_bd(0, USB_EP_TX, s->pingpong);

        /* XXX fairly inefficient assignment */
        bd->addr = addr;
        bd->bd = ((struct USB_BD_t) {
                        .bc = len,
                                .dts = 1,
                                .data01 = s->data01,
                                .own = 1
                                }).bd;
}

void
usb_rx_queue_next(struct usbd_ep_pipe_state_t *s, void *addr, size_t len)
{
        struct USB_BD_t *bd = usb_get_bd(0, USB_EP_RX, s->pingpong);

        /* XXX fairly inefficient assignment */
        bd->addr = addr;
        bd->bd = ((struct USB_BD_t) {
                        .bc = len,
                                .dts = 1,
                                .data01 = s->data01,
                                .own = 1
                                }).bd;
}
