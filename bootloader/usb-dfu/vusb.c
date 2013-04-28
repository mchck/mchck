#include <sys/socket.h>

#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include "usb.h"

void *
usb_get_xfer_data(struct usb_xfer_info *i)
{
}

enum usb_tok_pid
usb_get_xfer_pid(struct usb_xfer_info *i)
{
}

void
usb_enable_xfers(void)
{
}

void
usb_set_addr(int addr)
{
}

void
usb_ep_stall(int ep)
{
}

void
usb_clear_transfers(void)
{
}

inline size_t
usb_ep_get_transfer_size(int ep, enum usb_ep_dir dir, enum usb_ep_pingpong pingpong)
{
}

void
usb_tx_queue_next(struct usbd_ep_pipe_state_t *s)
{
}

void
usb_rx_queue_next(struct usbd_ep_pipe_state_t *s)
{
}


static int vusb_sock;

static void
vusb_attach(void)
{
        /**
         * Interfacing to usbip-host/vhci_hcd:
         *
         * 1. find free roothub port
         * 2. create socket to pass to the kernel
         * 3. attach
         */

        /* (1) find free roothub port */
        FILE *statusf = fopen("/sys/devices/platform/vhci_hcd/status", "r");

        if (statusf == NULL)
                errx(1, "usbip-host module not loaded?");

        int nextport = -1;

        /* skip header */
        fscanf(statusf, "%*[^\n]\n");
        while (!feof(statusf)) {
                int portid, status;
                if (fscanf(statusf, "%d %d %*[^\n]\n", &portid, &status) != 2)
                        warnx("odd vhci status line");

                if (status == 4) {
                        nextport = portid;
                        break;
                }
        }
        fclose(statusf);

        if (nextport < 0)
                errx(1, "no free port found");

        /* (2) create socket */
        int sockp[2];
        if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockp) < 0)
                err(1, "cannot create socket");

        /* (3) pass to kernel */
        /* XXX needs root priv */
        FILE *attachf = fopen("/sys/devices/platform/vhci_hcd/attach", "w");
        if (attachf == NULL)
                err(1, "cannot open attach to vhci");
        if (fprintf(attachf, "%u %u %u %u",
                    nextport,
                    sockp[0],
                    nextport + 1,   /* devid */
                    2               /* USB_SPEED_FULL */
                    ) < 0 ||
            fclose(attachf) < 0)
                err(1, "cannot attach to vhci");

        /* belongs to the kernel now */
        close(sockp[0]);

        vusb_sock = sockp[1];
}

static void
vusb_rcv(void)
{
        
}


int
main(void)
{
        vusb_attach();
        for (;;) {
                vusb_rcv();
        }
}
