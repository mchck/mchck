/*
 * Linker script for MK20DX32VLF5
 */

MEMORY
{
	loader_rom (rx) : ORIGIN = 0x0, LENGTH = 32K
	app_rom (rx) : ORIGIN = 32K, LENGTH = 0
	ram (rwx) : ORIGIN = 0x20000000 - 8K / 2, LENGTH = 8K
}

#include "K20.ld"
