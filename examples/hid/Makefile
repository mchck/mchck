PROG=	hid

ifndef VUSB
SRCS+=	khid.c
else
TARGET=	host
SRCS+=	vhid.c
endif

include ../../toolchain/mchck.mk
