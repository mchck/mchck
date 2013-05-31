PROG=	dfu

ifdef VUSB
include Makefile.vusb
else

SRCS+=	main-kinetis.c
CFLAGS+= -DSHORT_ISR
USE=	usb
NO_LTO= 1
LOADER=	1

include ../../toolchain/mchck.mk

endif
