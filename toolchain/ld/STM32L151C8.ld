/*
 * Linker script for STM32L151C8T6
 */

MEMORY
{
	app_rom (rx) : ORIGIN = 0x08000000, LENGTH = 64K
	loader_rom (rx) : ORIGIN = 0x1ff00000, LENGTH = 4K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 10K
}

_eram = 0x2000000 + 0x00002800;
