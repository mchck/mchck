_libdir:=       $(dir $(lastword ${MAKEFILE_LIST}))

-include .mchckrc
-include ${_libdir}/../.mchckrc

ifndef MAKECMDGOALS
.DEFAULT_GOAL:=
endif

is-make-clean=	$(filter clean realclean,${MAKECMDGOALS})


define _include_libs
_libdir-$(1):=	$$(addprefix $${_libdir}/lib/,$(1))
include $$(addsuffix /Makefile.part,$${_libdir-$(1)})

_forceobjs-$(1)=	$$(addsuffix .o, $$(basename $$(addprefix $(1)-lib-,$${SRCS.force-$(1)})))
FORCEOBJS+=	$${_forceobjs-$(1)}
_objs-$(1)=	$$(addsuffix .o, $$(basename $$(addprefix $(1)-lib-,$${SRCS-$(1)}))) $${_forceobjs-$(1)}
_allobjs+=	$${_objs-$(1)}
_libobjs+=	$${_objs-$(1)}
CLEANFILES+=	$${_objs-$(1)}
_deps-$(1)=	$$(addsuffix .d, $$(basename $$(addprefix $(1)-lib-,$${SRCS-$(1)} $${SRCS.force-$(1)})))
DEPS+=	$${_deps-$(1)}

$(1)-lib-%.o: $${_libdir-$(1)}/%.c
	$$(COMPILE.c) $$(OUTPUT_OPTION) $$<
$(1)-lib-%.d: $${_libdir-$(1)}/%.c
	$$(GENERATE.d)
endef

GENERATE.d=	$(CC) -MM ${CPPFLAGS} -MT $@ -MT ${@:.d=.o} -MP -MF $@ $<


# Common config

CPPFLAGS+=	-I${_libdir}/include -I${_libdir}/lib
CPPFLAGS+=	-std=gnu11
CFLAGS+=	-fplan9-extensions
CFLAGS+=	-ggdb3
ifndef DEBUG
CFLAGS+=	${COPTFLAGS}
else
NO_LTO=		no-lto
endif
CFLAGS+=	${CWARNFLAGS}

SRCS?=	${PROG}.c
_objs=	$(addsuffix .o, $(basename ${SRCS}))
CLEANFILES+=	${_objs}
OBJS+=	${_objs}
_allobjs+=	${OBJS}
DEPS+=	$(addsuffix .d, $(basename ${SRCS}))
CLEANFILES+=	${DEPS}

# Host config (VUSB)

ifeq (${TARGET},host)
CPPFLAGS+=	-DTARGET_HOST
CFLAGS+=	-fshort-enums

all: ${PROG}

$(foreach _uselib,host ${USE},$(eval $(call _include_libs,$(_uselib))))

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
GDB=	arm-none-eabi-gdb
DFUUTIL?=	dfu-util
RUBY?=	ruby

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
CFLAGS+=	-flto -fno-use-linker-plugin
endif
CPPFLAGS+=	-I${_libdir}/CMSIS/Include -I.
CPPFLAGS+=	-include ${_libdir}/include/mchck_internal.h

LDFLAGS+=	-Wl,--gc-sections
LDFLAGS+=	-fwhole-program
CPPFLAGS.ld+=	-P -CC -I${_libdir}/ld -I.
CPPFLAGS.ld+=	-DTARGET_LDSCRIPT='"${TARGETLD}"'
LDSCRIPTS+=	${_libdir}/ld/${TARGETLD}
TARGETLD?=	${TARGET}.ld

ifdef LOADER
CPPFLAGS.ld+=	-DMEMCFG_LDSCRIPT='"loader.ld"'
LDSCRIPTS+=	${_libdir}/ld/loader.ld
BINSIZE=	${LOADER_SIZE}
LOADADDR=	${LOADER_ADDR}
else
CPPFLAGS.ld+=	-DMEMCFG_LDSCRIPT='"app.ld"'
LDSCRIPTS+=	${_libdir}/ld/app.ld
BINSIZE=	${APP_SIZE}
LOADADDR=	${APP_ADDR}
endif

LDTEMPLATE=	${PROG}.ld-template
LDFLAGS+=	-T ${LDTEMPLATE}
LDFLAGS+=       -nostartfiles
LDFLAGS+=	-Wl,-Map=${PROG}.map
LDFLAGS+=	-Wl,-output-linker-script=${PROG}.ld


CLEANFILES+=	${PROG}.hex ${PROG}.elf ${PROG}.bin ${PROG}.map

all: ${PROG}.bin

# This has to go before the rule, because for some reason the updates to OBJS
# are not incorporated into the target dependencies
include ${_libdir}/lib/Makefile.part
$(foreach _uselib,${SRCS.libs},$(eval $(call _include_libs,$(_uselib))))

# linkdep defines LINKOBJS
include ${_libdir}/mk/linkdep.mk

${PROG}.elf: ${LINKOBJS} ${LDLIBS} ${LDTEMPLATE}
	${CC} -o $@ ${CFLAGS} ${LDFLAGS} ${LINKOBJS} ${LDLIBS}

%.bin: %.elf
	${OBJCOPY} -O binary $< $@.tmp
	ls -l $@.tmp | awk '{ s=$$5; as=${BINSIZE}; printf "%d bytes available\n", (as - s); if (s > as) { exit 1; }}'
	mv $@.tmp $@
CLEANFILES+=	${PROG}.bin.tmp

${LDTEMPLATE}: ${_libdir}/ld/link.ld.S ${LDSCRIPTS}
	${CPP} -o $@ ${CPPFLAGS.ld} $<
CLEANFILES+=	${LDTEMPLATE} ${PROG}.ld

gdb: ${PROG}.elf
	${RUBY} ${_libdir}/../programmer/gdbserver.rb ${MCHCKADAPTER} -- ${GDB} -readnow -ex 'target extended-remote :1234' ${PROG}.elf

flash: ${PROG}.bin
	${DFUUTIL} -D ${PROG}.bin

swd-flash: ${PROG}.bin
	${RUBY} ${_libdir}/../programmer/flash.rb ${MCHCKADAPTER} $< ${LOADADDR}
endif

# from the make info manual
%.d: %.c
	$(GENERATE.d)

ifeq ($(call is-make-clean),)
-include $(patsubst %.o,%.d,${LINKOBJS})
endif

clean:
	-rm -f ${CLEANFILES}

realclean:
	-rm -f ${REALCLEANFILES} ${CLEANFILES}
