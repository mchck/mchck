PROG=	crc

ifndef VUSB
SRCS+=	main-kinetis.c
else
TARGET=	host
SRCS+=	vcdc.c
endif

SRCS+=	crc.desc

include ../../toolchain/mchck.mk
