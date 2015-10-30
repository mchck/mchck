/*
 * Linker script for MK20DX32VLF5
 */

MEMORY
{
/**
 * Our two-stage optimizing linker has trouble when the first link stage
 * runs out of space.  We artificially keep this size too large for now.
 * The overflow will be caught later.
 */

        loader_rom (rx) : ORIGIN = LOADER_ADDR, LENGTH = 1M
        app_rom (rx) : ORIGIN = APP_ADDR, LENGTH = 1M
        ram (rwx) : ORIGIN = 0x20000000 - 8K / 2, LENGTH = 8K
}

#include "K20.ld"
