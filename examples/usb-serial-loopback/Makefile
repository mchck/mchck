PROG=	usb-serial-loopback

ifndef VUSB
SRCS+=	main-kinetis.c
else
TARGET=	host
SRCS+=	vcdc.c
endif

SRCS+=	usb-serial-loopback.desc

include ../../toolchain/mchck.mk
