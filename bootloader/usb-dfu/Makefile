PROG=	dfu

CFLAGS+= -DSHORT_ISR
USE=	usb
#NO_LTO= 1
LOADER=	1

ifndef VUSB
SRCS+=	main-kinetis.c
else
TARGET=	host
SRCS+=	vdfu.c
endif

include ../../toolchain/mchck.mk
