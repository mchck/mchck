_libdir:=       $(dir $(lastword ${MAKEFILE_LIST}))
VPATH:=	${_libdir}/crt0:${_libdir}/lib:$(VPATH)

CC=	arm-none-eabi-gcc
LD=	arm-none-eabi-ld
AR=	arm-none-eabi-ar
AS=	arm-none-eabi-as
OBJCOPY=	arm-none-eabi-objcopy

ifeq ($(shell which $(CC) 2>/dev/null),)
SATDIR?=	$(HOME)/sat
endif
ifdef SATDIR
PATH:=	${SATDIR}/bin:${PATH}
export PATH
endif

TARGET?=	MK20DX32VLF5

COPTFLAGS?=	-Os
CWARNFLAGS?=	-Wall -Wno-main

CFLAGS+=	-mcpu=cortex-m4 -msoft-float -mthumb -ffunction-sections -std=gnu99
CFLAGS+=	-I${_libdir}/include -I${_libdir}/CMSIS/Include -I.
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

STARTFILE_SRC=	startup_k20.c system_k20.c
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
