/*
 * Linker script for Mini54LAN
 */

MEMORY
{
    	rom (rx) : ORIGIN = 0x00000000, LENGTH = 0x00004000
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00000800
}

_eram = 0x20000000 + 0x00000800;
