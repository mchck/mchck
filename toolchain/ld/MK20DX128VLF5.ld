/*
 * Linker script for MK20DX128VLF5
 */

MEMORY
{
        loader_rom (rx) : ORIGIN = LOADER_ADDR, LENGTH = LOADER_SIZE
        app_rom (rx) : ORIGIN = APP_ADDR, LENGTH = APP_SIZE
	ram (rwx) : ORIGIN = 0x20000000 - 16K / 2, LENGTH = 16K
}

#include "K20.ld"
