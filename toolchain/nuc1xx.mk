_libdir:=       $(dir $(lastword ${MAKEFILE_LIST}))
VPATH=	${_libdir}/crt0:${_libdir}/lib

CC=	arm-none-eabi-gcc
LD=	arm-none-eabi-ld
AR=	arm-none-eabi-ar
AS=	arm-none-eabi-as
OBJCOPY=	arm-none-eabi-objcopy

XTALFREQ?=	8000000
TARGET?=	NUC120LD2BN

CFLAGS+=	-mcpu=cortex-m0 -msoft-float -mthumb -ffunction-sections -std=gnu99
CFLAGS+=	-I${_libdir}/include -I${_libdir}/include/Driver
CFLAGS+=	-D${TARGET}
CFLAGS+=	-D__XTAL='(${XTALFREQ}L)'

LDFLAGS+=	-Wl,--gc-sections -L${_libdir}/ld -T ${TARGETLD} -T link.ld -nostdlib
#-nostdlib -nostartfiles -nodefaultlibs
LDLIBS+=	-lgcc

STARTFILE_SRC=	core_cm0.c system_NUC1xx.c startup_coide.c
STARTFILE_OBJ=	$(addsuffix .o, $(basename ${STARTFILE_SRC}))
STARTFILE_LIB=	libcrtnuc1xx.a
STARTFILE_LIBSHORT=	-lcrtnuc1xx
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
	${CC} -o $@ ${LDFLAGS} ${STARTFILES} ${OBJS} ${LDLIBS}

%.hex: %.elf
	${OBJCOPY} -O ihex $< $@

%.bin: %.elf
	${OBJCOPY} -O binary $< $@

${STARTFILE_LIB}: ${STARTFILE_OBJ}
	${AR} r $@ $^

clean:
	-rm -f ${CLEANFILES}
