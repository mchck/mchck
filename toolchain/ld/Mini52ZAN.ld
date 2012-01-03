/*
 * Linker script for Mini52ZAN
 */

MEMORY
{
    	rom (rx) : ORIGIN = 0x00000000, LENGTH = 0x00002000
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00000800
}

_eram = 0x20000000 + 0x00000800;
