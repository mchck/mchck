TARGET = swd-msp430.elf
MCU = msp430g2553

OBJECTS = main.o ../swduino/swd.o

CFLAGS = -Os -Wall -g -mmcu=$(MCU)

CC = msp430-gcc
PROGRAM = mspdebug rf2500

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

flash:
	$(PROGRAM) "prog $(TARGET)"

clean:
	rm -fr $(OBJECTS) $(TARGET)
