/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
 *
 * This file is part of the BeeOS software.
 *
 * BeeOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BeeOS; if not, see <http://www.gnu/licenses/>.
 */

/*
 * Multiboot compatible entry point.
 */

#include "vmem.h"
#include "paging_bits.h"

#define MAGIC           0x1BADB002
#define PAGE_ALIGN      (1 << 0)
#define MEMORY_INFO     (1 << 1)
#define FLAGS           (PAGE_ALIGN | MEMORY_INFO)
#define CHECKSUM        -(MAGIC+FLAGS)

.intel_syntax noprefix

.extern arch_init

.section .text

/* 
 * Kernel entry
 */
.global startup
.type startup, @function
startup:
    jmp     1f

/* Multiboot header */
.align 4
    .long   MAGIC
    .long   FLAGS
    .long   CHECKSUM

1:  cli     /* Turn off interrupts */
    cld     /* Clear direction flag */
    
    /*
     * Paging early initialization.
     * Create a temporary 4MB page table used to hold the initial
     * kernel code and data. Higher level C code will properly
     * initialize the page directory with 4KB tables hierarchy.
     * Paging is vital because all the kernel has been linked as
     * a upper half kernel (0xC0000000 + load address).
     */

    /* Turn on page size extension for 4MB pages */
    mov     eax, cr4  
    or      eax, CR4_PSE
    mov     cr4, eax
    /* Initialize the page directory relevant entries */
    mov     eax, offset (kpage_dir - KVBASE)
    mov     dword ptr [eax], (PTE_P|PTE_W|PTE_PS)
    mov     dword ptr [eax+4*768], (PTE_P|PTE_W|PTE_PS)
    mov     cr3, eax
    /* Turn on paging */
    mov     eax, cr0
    or      eax, (CR0_PG|CR0_WP)
    mov     cr0, eax
    mov     eax, offset 2f
    jmp     eax
    
    /* Higher half start */
2:  mov     esp, offset (kstack+PAGE_SIZE)
    mov     ebp, 0  /* stack backtrace stop condition */

    push    0
    popf            /* clear the eflags register */

    /* Low level initialization using multiboot data */
    add     ebx, KVBASE  /* Adjust multiboot info address */
    push    ebx
    call    arch_init
    add     esp, 4

    /* Enter the kernel main */
    call    kmain
    jmp     freeze      /* Should never return */
.size startup, .-startup

/*
 * Freeze routine.
 * Clears interrupts and halts the processor.
 */
.global freeze
.type freeze, @function
freeze:
    cli
	hlt
	jmp    freeze  /* just in case... */
.size freeze, .-freeze

/*
 * Utility function to fetch the next instruction address.
 * The address is fetched reading the address pushed into the
 * stack by the 'call' instruction.
 */
.global get_eip
.type get_eip, @function
get_eip:
    mov     eax, [esp]
    ret
.size get_eip, .-get_eip

/*
 * Miscelanea uninitialized data
 */

.section .bss

/* 
 * Kernel first task (pid=0) page directory.
 */
.global kpage_dir
.type kpage_dir, @object
.align PAGE_SIZE
kpage_dir:
.space PAGE_SIZE
.size kpage_dir, .-kpage_dir

/* 
 * Kernel first task (pid=0) stack.
 * The stack is defined to have the size of a page.
 */
.global kstack
.type kstack, @object
.align PAGE_SIZE
kstack:
.space PAGE_SIZE
.size kstack, .-kstack
