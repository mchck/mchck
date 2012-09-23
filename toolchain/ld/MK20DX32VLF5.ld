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
