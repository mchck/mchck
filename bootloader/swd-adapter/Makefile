PROG=	swd-adapter
USE=	usb

SRCS=	main-mchck.c

# Arduino requires the sketch to be in a directory of the same name,
# and all required files to be in the same directory.
# So there we go, putting our source into the ardunio sketch directory,
# so that the IDE can compile it.
SRCS+=	swduino/swd.c

include ../../toolchain/mchck.mk
