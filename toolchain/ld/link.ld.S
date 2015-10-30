/* -*- mode: ld-script; -*- */

#include TARGET_LDSCRIPT
#include MEMCFG_LDSCRIPT

/* PACK SECTIONS IGNORE .ramtext.* */

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

ENTRY(Reset_Handler)

/* Section Definitions */
SECTIONS
{
    .text :
    {
/*
 * pack-sections.rb will replace the following block
 * with sections packed and ordered according to the
 * positioning restrictions defined in the TARGET_LDSCRIPT.
 */
/* PACK SECTIONS START */
        KEEP(*(.isr_vector .isr_vector.*))
        KEEP(*(.flash_config))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
/* PACK SECTIONS END */
    } > rom

#ifdef TARGET_SECTIONS_LDSCRIPT
#include TARGET_SECTIONS_LDSCRIPT
#endif

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > rom

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > rom

    . = ALIGN(4);
    _etext = .;

    /* The USB BDT has to be aligned to a 512 byte boundary */
    .usb_bdt (NOLOAD) :
    {
        . = ALIGN(512);
        _sbss = . ;
        *(.usb_bdt)
    } > ram

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        *(SORT_BY_ALIGNMENT(.bss*) SORT_BY_ALIGNMENT(COMMON*))
        . = ALIGN(4);
        _ebss = . ;
    } > ram

    _sidata = LOADADDR(.data);

    .data : AT (_etext)
    {
        _sdata = .;
        *(SORT_BY_ALIGNMENT(.ramtext.*) SORT_BY_ALIGNMENT(.data*))
        . = ALIGN(4);
        _edata = . ;
    } > ram

    /* stack section */
    .co_stack (NOLOAD):
    {
        . = ALIGN(8);
        *(.co_stack .co_stack.*)
    } > ram

    . = ALIGN(4);
    _end = . ;
}
