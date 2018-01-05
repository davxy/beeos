OUTPUT_FORMAT("elf32-littlearm","elf32-bigarm","elf32-littlearm")

ENTRY(startup)

SECTIONS
{
    . = 0x100000;

    .text :
    {
        *startup.o(.text)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    }

    .data :
    {
        data = .;
        *(.data)
        . = ALIGN(4);
    }
  
    .bss :
    {
        bss = .;
        *(.bss)
        . = ALIGN(4);
    }
  
    kend = .;
}
