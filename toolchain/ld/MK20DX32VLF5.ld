/*
 * Linker script for MK20DX32VLF5
 */

MEMORY
{
	loader_rom (rx) : ORIGIN = 0x0, LENGTH = 3K
	app_rom (rx) : ORIGIN = 3K, LENGTH = 29K
	ram (rwx) : ORIGIN = 0x20000000 - 8K / 2, LENGTH = 8K
}

_app_rom = ORIGIN(app_rom);
_eram = ORIGIN(ram) + LENGTH(ram);

/* PACK SECTIONS FIX .isr_vector = 0 */
/* PACK SECTIONS FIX .flash_config = 0x400 */

/* peripherals */
FTFL_CONFIG = 0x400;
FlexRAM = 0x14000000;
FTFL = 0x40020000;
SIM = 0x40047000;
MCG = 0x40064000;
USB0 = 0x40072000;
RCM = 0x4007f000;

PORTA = 0x40049000;
PORTB = 0x4004A000;
PORTC = 0x4004B000;
PORTD = 0x4004C000;
PORTE = 0x4004D000;
