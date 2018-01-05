OUTPUT_FORMAT("elf32-littlearm","elf32-bigarm","elf32-littlearm")

ENTRY(startup)

/* memory map of AT91SAM7S64 */
MEMORY
{
    ROM (rx)  : ORIGIN = 0x00100000, LENGTH = 64k
    RAM (rwx) : ORIGIN = 0x00200000, LENGTH = 16k
}

SECTIONS
{
    .text :
    {
        *startup.o(.text)
        *(.text*)
        *(.rodata*)
    } > ROM

    /* for vectors remapped to RAM */
    .ramvect :
    {
        __ram_start = .;
        . = 0x40;
    } > RAM

    .data :
    {
        data = .;
        *(.data)
        . = ALIGN(4);
    } > RAM
  
    .bss :
    {
        bss = .;
        *(.bss)
        . = ALIGN(4);
    } > RAM
  
    kend = .;
}
