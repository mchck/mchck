#PROG=printf
#include ../../toolchain/mchck.mk

CFLAGS+=	-Wall -O0 -g -m32 -DTEST_PRINTF=1 -fplan9-extensions -std=gnu11
LDFLAGS+=	-m32

all: printf
.PHONY: all

printf: printf.o

test: all
	./printf
