PROG=	usb-serial-loopback
USE=	usb

ifndef VUSB
SRCS+=	main-kinetis.c
else
TARGET=	host
SRCS+=	vcdc.c
endif

include ../../toolchain/mchck.mk
