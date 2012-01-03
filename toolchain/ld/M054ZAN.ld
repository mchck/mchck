/*
 * Linker script for M054ZAN
 */

MEMORY
{
    	rom (rx) : ORIGIN = 0x00000000, LENGTH = 0x00004000
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00001000
}

_eram = 0x20000000 + 0x00001000;
