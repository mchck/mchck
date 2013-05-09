CFLAGS+= -Wall -std=c11 -g -DSHORT_ISR

ifndef ARCH
all: vusb mchck

vusb mchck::
	mkdir -p obj-$@
	make ARCH=$@ SRCDIR=$(CURDIR) -C obj-$@ -f $(CURDIR)/Makefile

clean:
	-rm -rf obj-*
else

VPATH=	$(SRCDIR)
PROG=	dfu
SRCS=	usb.c dfu.c
LOADER=	1

ifeq ($(ARCH),mchck)
SRCS+=	usb-kinetis.c main-kinetis.c

include $(SRCDIR)/../../toolchain/mchck.mk
endif

ifeq ($(ARCH),vusb)
SRCS+=	vusb.c vdfu.c

OBJS+=	$(addsuffix .o, $(basename $(SRCS)))
$(PROG): $(OBJS)
endif
endif
