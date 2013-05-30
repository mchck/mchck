PROG=	dfu
SRCS=	usb.c dfu.c
CFLAGS+= -Wall -std=c11 -g

ifdef VUSB
include Makefile.vusb
else

SRCS+=	usb-kinetis.c main-kinetis.c
CFLAGS+= -DSHORT_ISR
NO_LTO= 1
LOADER=	1

include ../../toolchain/mchck.mk

endif
