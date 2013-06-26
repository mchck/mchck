_libdir:=       $(dir $(lastword ${MAKEFILE_LIST}))

define _include_used_libs
_libdir-$(1):=	$$(addprefix $${_libdir}/lib/,$(1))
include $$(addsuffix /Makefile.part,$${_libdir-$(1)})

_objs-$(1)=	$$(addsuffix .o, $$(basename $$(addprefix $(1)-lib-,$${SRCS-$(1)})))
OBJS+=	$${_objs-$(1)}
CLEANFILES+=	$${_objs-$(1)}

$(1)-lib-%.o: $${_libdir-$(1)}/%.c
	$$(COMPILE.c) $$(OUTPUT_OPTION) $$<
endef


# Common config

CFLAGS+=	-I${_libdir}/include -I${_libdir}/lib
CFLAGS+=	-g
CFLAGS+=	-std=c11 -fplan9-extensions
ifndef DEBUG
CFLAGS+=	${COPTFLAGS}
endif
CFLAGS+=	${CWARNFLAGS}

SRCS?=	${PROG}.c
_objs=	$(addsuffix .o, $(basename ${SRCS}))
CLEANFILES+=	${_objs}
OBJS+=	${_objs}


# Host config (VUSB)

ifeq (${TARGET},host)
CFLAGS+=	-DTARGET_HOST
CFLAGS+=	-fshort-enums

all: ${PROG}

$(foreach _uselib,${USE},$(eval $(call _include_used_libs,$(_uselib))))

$(PROG): $(OBJS)
	$(LINK.c) $^ ${LDLIBS} -o $@

CLEANFILES+=	${PROG}
else

# MCHCK config

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

CFLAGS+=	-mcpu=cortex-m4 -msoft-float -mthumb -ffunction-sections -fdata-sections -fno-builtin -fstrict-volatile-bitfields
ifndef NO_LTO
CFLAGS+=	-flto
endif
CFLAGS+=	-I${_libdir}/CMSIS/Include -I.
CFLAGS+=	-include ${_libdir}/include/mchck_internal.h

LDFLAGS+=	-Wl,--gc-sections
LDFLAGS+=	-fwhole-program
CPPFLAGS.ld+=	-P -CC -I${_libdir}/ld -I.
CPPFLAGS.ld+=	-DTARGET_LDSCRIPT='"${TARGETLD}"'
LDSCRIPTS+=	${_libdir}/ld/${TARGETLD}
TARGETLD?=	${TARGET}.ld

ifdef LOADER
CPPFLAGS.ld+=	-DMEMCFG_LDSCRIPT='"loader.ld"'
LDSCRIPTS+=	${_libdir}/ld/loader.ld
else
CPPFLAGS.ld+=	-DMEMCFG_LDSCRIPT='"app.ld"'
LDSCRIPTS+=	${_libdir}/ld/app.ld
endif

LDFLAGS+=	-T ${PROG}.ld
LDFLAGS+=       -nostartfiles


CLEANFILES+=	${PROG}.hex ${PROG}.elf ${PROG}.bin

all: ${PROG}.hex ${PROG}.bin

# This has to go before the rule, because for some reason the updates to OBJS
# are not incorporated into the target dependencies
$(foreach _uselib,mchck ${USE},$(eval $(call _include_used_libs,$(_uselib))))


${PROG}.elf: ${OBJS} ${LDLIBS} ${PROG}.ld
	${CC} -o $@ ${CFLAGS} ${LDFLAGS} ${OBJS} ${LDLIBS}

%.hex: %.elf
	${OBJCOPY} -O ihex $< $@

%.bin: %.elf
	${OBJCOPY} -O binary $< $@

${PROG}.ld: ${_libdir}/ld/link.ld.S ${LDSCRIPTS}
	${CPP} -o $@ ${CPPFLAGS.ld} $<
CLEANFILES+=	${PROG}.ld
endif

clean:
	-rm -f ${CLEANFILES}
