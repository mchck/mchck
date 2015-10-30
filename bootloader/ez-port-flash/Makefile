PROG=	ez-port-flash
SRCS=	ez-port-flash.rl
USE=	usb
OBJS=	payload.o
DEFAULT_PAYLOAD=	../usb-dfu/dfu.bin ../../examples/blink/blink.bin
PAYLOAD?=	${DEFAULT_PAYLOAD}
CLEANFILES+=	payload.o payload.bin

payload.o: payload.bin
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm \
		--set-section-flags .data=alloc,contents,load,readonly,data \
		--rename-section .data=.rodata.payload $< $@

payload.bin: ${PAYLOAD}
	for f in $^; do \
		dd if=$$f bs=1024 conv=sync; \
	done > $@

${DEFAULT_PAYLOAD}::
	cd $(dir $@) && make DEBUG=

_gensrc+=	$(addsuffix .c,$(basename $(filter %.rl,${SRCS})))
%.c: %.rl
	ragel -C -G0 -o $@ $<
CLEANFILES+=	$(patsubst %.rl,%.c,$(filter %.rl,${SRCS}))

include ../../toolchain/mchck.mk
