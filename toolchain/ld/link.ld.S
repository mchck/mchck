/* -*- mode: ld-script; -*- */

#include TARGET_LDSCRIPT
#include MEMCFG_LDSCRIPT

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/* Section Definitions */
SECTIONS
{
    .text :
    {
#ifdef TEXT_LDSCRIPT
#include TEXT_LDSCRIPT
#else
        KEEP(*(.isr_vector .isr_vector.*))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
#endif
    } > rom

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
        *(SORT_BY_ALIGNMENT(.bss) SORT_BY_ALIGNMENT(COMMON))
        . = ALIGN(4);
        _ebss = . ;
    } > ram

    _sidata = .;

    .data : AT (_etext)
    {
        _sdata = .;
        *(SORT_BY_ALIGNMENT(.data*))
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
