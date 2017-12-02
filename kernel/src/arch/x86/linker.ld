OUTPUT_FORMAT("elf32-i386")

ENTRY(startup)

virt = 0xC0100000;
phys = 0x00100000;


SECTIONS
{ 
    . = virt;

    kstart = .;
    
    .text : AT(phys) 
    {
        code = .;
        *startup.o(.text)
        *(.text)
        *(.rodata)
        . = ALIGN(4096);
    }
  
    .data : AT(phys + (data - code))
    {
        data = .;
        *(.data)
        . = ALIGN(4096);
    }
  
    .bss : AT(phys + (bss - code))
    {
        bss = .;
        *(.bss)
        . = ALIGN(4096);
    }
  
    kend = .;
}
