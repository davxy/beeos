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
 * User mode entry point.
 */

#include "vmem.h"
#include <unistd.h>

.intel_syntax noprefix

.section .text

/*
 * This stub must be position independent.
 * The kernel relocates it at UVADDR (a valid user mode address) and
 * is executed in user mode (ring 3). This stub just executes the 'init'
 * program assumed to be located under the /sbin' directory under the
 * root file system. The program is executed via the 'execve' syscall
 * facility.
 */
userstub_start:
    xor     edx, edx        /* Set envp as null */
    push    edx             /* Push a null word */
    jmp     1f              /* Get the string address */
2:  mov     ebx, [esp]      /* Get string address in ebx */
    mov     ecx, esp        /* Get address of argv array */
    mov     eax, __NR_execve
    int     0x80
    mov     eax, __NR_exit  /* On error, just exit */
    int     0x80
1:  call    2b              /* Push string address */
    .asciz "/sbin/init"
userstub_end:

/*
 * Init process startup.
 * Loads the user-mode code stub and jump to it with user provileges.
 * The stub must be loaded below the kernel virtual base address.
 */
.extern page_map
.global init
init:
    /* Map an arbitrary userspace page */
    push    -1          /* let page_map allocate the phys frame */
    push    UVADDR      /* virtual address */
    call    page_map
    pop     edi
    add     esp, 4 
    cmp     eax, -1
    jne     1f
    ret
    /* Copy the user stub to the user page */ 
1:  mov     esi, offset userstub_start
    mov     ecx, offset userstub_end
    sub     ecx, esi    /* get stub size */
    cld
    rep     movsb
    mov     ax, 0x23    /* user data segment selector */
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax      /* ss is handled by iret */
    push    0x23        /* user data segment selector (for ss) */
    push    UVADDR+0x1000   /* temporary user stack */
    push    0x200       /* eflags = IF (enable interrupts) */
    push    0x1B        /* user code segment selector */
    push    UVADDR      /* return address */
    iret
