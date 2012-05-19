_libdir:=       $(dir $(lastword ${MAKEFILE_LIST}))
VPATH=	${_libdir}/crt0:${_libdir}/lib

CC=	arm-none-eabi-gcc
LD=	arm-none-eabi-ld
AR=	arm-none-eabi-ar
AS=	arm-none-eabi-as
OBJCOPY=	arm-none-eabi-objcopy

XTALFREQ?=	8000000
TARGET?=	STM32L151C8

COPTFLAGS?=	-Os
CWARNFLAGS?=	-Wall

CFLAGS+=	-mcpu=cortex-m3 -msoft-float -mthumb -ffunction-sections -std=gnu99
CFLAGS+=	-I${_libdir}/include -I.
CFLAGS+=	-D__XTAL='(${XTALFREQ}L)'
CFLAGS+=	-include ${_libdir}/include/mchck_internal.h
CFLAGS+=	-g
ifndef DEBUG
CFLAGS+=	${COPTFLAGS}
endif
CFLAGS+=	${CWARNFLAGS}

LDFLAGS+=	-Wl,--gc-sections -L${_libdir} -L${_libdir}/ld
LDFLAGS+=	-T ${TARGETLD}
ifdef LOADER
LDFLAGS+=	-T loader.ld
else
LDFLAGS+=	-T app.ld
endif
LDFLAGS+=	-T link.ld
LDFLAGS+=       -nostartfiles

STARTFILE_SRC=	startup_stm32l15xxx.c system_stm32l1xx.c
STARTFILE_OBJ=	$(addsuffix .o, $(basename ${STARTFILE_SRC}))
#STARTFILE_LIB=	libcrtnuc1xx.a
#STARTFILE_LIBSHORT=	-lcrtnuc1xx
STARTFILES=	${STARTFILE_OBJ}

TARGETLD?=	${TARGET}.ld

SRCS?=	${PROG}.c
_objs=	$(addsuffix .o, $(basename ${SRCS}))
CLEANFILES+=	${_objs}
OBJS+=	${_objs}

CLEANFILES+=	${STARTFILE_OBJ}

CLEANFILES+=	${PROG}.hex ${PROG}.elf ${PROG}.bin

all: ${PROG}.hex ${PROG}.bin

${PROG}.elf: ${OBJS} ${STARTFILES}
	${CC} -o $@ ${CFLAGS} ${LDFLAGS} ${STARTFILES} ${OBJS} ${LDLIBS}

%.hex: %.elf
	${OBJCOPY} -O ihex $< $@

%.bin: %.elf
	${OBJCOPY} -O binary $< $@

${STARTFILE_LIB}: ${STARTFILE_OBJ}
	${AR} r $@ $^

clean:
	-rm -f ${CLEANFILES}
