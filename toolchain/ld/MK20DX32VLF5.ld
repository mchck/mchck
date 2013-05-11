/*
 * Linker script for MK20DX32VLF5
 */

MEMORY
{
	loader_rom (rx) : ORIGIN = 0x0, LENGTH = 2K
	app_rom (rx) : ORIGIN = 2K, LENGTH = 30K
	ram (rwx) : ORIGIN = 0x20000000 - 8K / 2, LENGTH = 8K
}

_eram = ORIGIN(ram) + LENGTH(ram);

/* registers */
USB0_CTL = 0x40072094;
USB0_ADDR = 0x40072098;
USB0_ENDPT = 0x400720c0;

FTFL = 0x40020000;
