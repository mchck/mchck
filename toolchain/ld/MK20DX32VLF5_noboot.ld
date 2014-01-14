/*
 * Linker script for MK20DX32VLF5
 */

MEMORY
{
	loader_rom (rx) : ORIGIN = 0x0, LENGTH = 32K
	app_rom (rx) : ORIGIN = 32K, LENGTH = 0
	ram (rwx) : ORIGIN = 0x20000000 - 8K / 2, LENGTH = 8K
}

_app_rom = ORIGIN(app_rom);
_eram = ORIGIN(ram) + LENGTH(ram);

/* PACK SECTIONS FIX .isr_vector = 0 */
/* PACK SECTIONS FIX .flash_config = 0x400 */

/* peripherals */
FTFL_CONFIG = 0x00000400;
FlexRAM     = 0x14000000;

DMA         = 0x40008000;
DMAMUX      = 0x40021000;
FTFL        = 0x40020000;
SPI0        = 0x4002c000;
I2S0        = 0x4002f000;
CRC         = 0x40032000;
PIT         = 0x40037000;
FTM0        = 0x40038000;
FTM1        = 0x40039000;
ADC0        = 0x4003b000;
RTC         = 0x4003d000;
LPTMR0      = 0x40040000;
TSI0        = 0x40045000;
SIM         = 0x40047000;

PORTA       = 0x40049000;
PORTB       = 0x4004a000;
PORTC       = 0x4004b000;
PORTD       = 0x4004c000;
PORTE       = 0x4004d000;

MCG         = 0x40064000;
I2C0        = 0x40066000;

UART0       = 0x4006A000;
UART1       = 0x4006B000;
UART2       = 0x4006C080;

USB0        = 0x40072000;
LLWU        = 0x4007c000;
PMC         = 0x4007d000;
SMC         = 0x4007e000;
RCM         = 0x4007f000;

GPIOA       = 0x400ff000;
GPIOB       = 0x400ff040;
GPIOC       = 0x400ff080;
GPIOD       = 0x400ff0C0;
GPIOE       = 0x400ff100;

SYSTICK     = 0xe000e010;
NVIC        = 0xe000e100;
SCB         = 0xe000ed00;
