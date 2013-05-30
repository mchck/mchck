_libdir:=       $(dir $(lastword ${MAKEFILE_LIST}))
VPATH:=	${_libdir}/crt0:${_libdir}/lib:${_libdir}/ld:${_libdir}:$(VPATH)

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

COMPILER_PATH=	${_libdir}/scripts
export COMPILER_PATH

TARGET?=	MK20DX32VLF5

include ${_libdir}/${TARGET}.mk

COPTFLAGS?=	-Os
CWARNFLAGS?=	-Wall -Wno-main

CFLAGS+=	-mcpu=cortex-m4 -msoft-float -mthumb -ffunction-sections -fdata-sections -std=c11 -fplan9-extensions -fno-builtin
ifndef NO_LTO
CFLAGS+=	-flto
endif
CFLAGS+=	-I${_libdir}/include -I${_libdir}/CMSIS/Include -I.
CFLAGS+=	-include ${_libdir}/include/mchck_internal.h
CFLAGS+=	-g
ifndef DEBUG
CFLAGS+=	${COPTFLAGS}
endif
CFLAGS+=	${CWARNFLAGS}

LDFLAGS+=	-Wl,--gc-sections
LDFLAGS+=	-fwhole-program
CPPFLAGS.ld+=	-P -CC -I${_libdir}/ld -I.
CPPFLAGS.ld+=	-DTARGET_LDSCRIPT='"${TARGETLD}"'

ifdef LOADER
STARTFILE_SRC+=	flashconfig_k20.c
CPPFLAGS.ld+=	-DMEMCFG_LDSCRIPT='"loader.ld"'
LDSCRIPTS+=	loader.ld
else
CPPFLAGS.ld+=	-DMEMCFG_LDSCRIPT='"app.ld"'
LDSCRIPTS+=	app.ld
endif

LDFLAGS+=	-T ${PROG}.ld
LDFLAGS+=       -nostartfiles

STARTFILE_SRC+=	startup_k20.c system_k20.c mchck-builtins.c
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

${PROG}.elf: ${OBJS} ${STARTFILES} ${LDLIBS} ${PROG}.ld
	${CC} -o $@ ${CFLAGS} ${LDFLAGS} ${STARTFILES} ${OBJS} ${LDLIBS}

%.hex: %.elf
	${OBJCOPY} -O ihex $< $@

%.bin: %.elf
	${OBJCOPY} -O binary $< $@

${STARTFILE_LIB}: ${STARTFILE_OBJ}
	${AR} r $@ $^

${PROG}.ld: link.ld.S ${LDSCRIPTS}
	cpp -o $@ ${CPPFLAGS.ld} $<
CLEANFILES+=	${PROG}.ld

clean:
	-rm -f ${CLEANFILES}
