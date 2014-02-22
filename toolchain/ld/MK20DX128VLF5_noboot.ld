/*
 * Linker script for MK20DX128VLF5 without bootloader
 */

MEMORY
{
	loader_rom (rx) : ORIGIN = 0x0, LENGTH = 128K
	app_rom (rx) : ORIGIN = 128K, LENGTH = 0
	ram (rwx) : ORIGIN = 0x20000000 - 16K / 2, LENGTH = 16K
}

#include "K20.ld"
