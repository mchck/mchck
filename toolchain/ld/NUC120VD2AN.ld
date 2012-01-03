/*
 * Linker script for NUC120VD2AN
 */

MEMORY
{
    	rom (rx) : ORIGIN = 0x00000000, LENGTH = 0x00010000
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00002000
}

_eram = 0x20000000 + 0x00002000;
