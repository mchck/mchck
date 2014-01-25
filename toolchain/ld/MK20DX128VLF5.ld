/*
 * Linker script for MK20DX128VLF5
 */

MEMORY
{
	loader_rom (rx) : ORIGIN = 0x0, LENGTH = 128K
	app_rom (rx) : ORIGIN = 3K, LENGTH = 128K-3K
	ram (rwx) : ORIGIN = 0x20000000 - 16K / 2, LENGTH = 16K
}

#include "K20.ld"
