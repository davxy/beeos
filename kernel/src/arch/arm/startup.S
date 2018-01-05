/*
 * Copyright (c) 2015-2016, Davide Galassi. All rights reserved.
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

#define KSTACK_SIZE     4096


.extern low_level_init
.extern kmain

.section .text

.code 32

.global startup
startup:
    /* Vector table, used briefly until RAM is remapped to address zero */
    b   reset   /* Reset: relative branch allows remap */
    b   .       /* Undefined instruction */
    b   .       /* Software interrupt */
    b   .       /* Prefetch abort */
    b   .       /* Data abort */
    b   .       /* Reserved */
    b   .       /* IRQ */
    b   .       /* FIQ */

reset:
    ldr     r0, =reset
    ldr     r1, =cstartup
    mov     lr, r1
    ldr     sp, =(kstack+KSTACK_SIZE)
    b       low_level_init

cstartup:
#if 0
    /* Relocate .fastcode section from ROM to RAM */
    ldr     r0, =fastcode_load
    ldr     r1, =fastcode_start
    ldr     r2, =fastcode_end
1:  cmp     r1, r2
    ldmltia r0!, {r3}
    stmltia r1!, {r3}
    blt     1b

    /* Relocate .data section from ROM to RAM */
    ldr     r0, =data_load
    ldr     r1, =data_start
    ldr     r2, =data_end
1:  cmp     r1, r2
    ldmltia r0!, {r3}
    stmltia r1!, {r3}
    blt     1b

    /* Clear the .bss section */
    ldr     r1, =bss_start
    ldr     r2, =bss_end
    mov     r3, #0
1:  cmp     r1, r2
    stmltia r1!, {r3}
    blt     1b

    /* Initialize the stack pointers for all ARM modes */
    msr     cpsr_c, #(IRQ_MODE | I_BIT | F_BIT)
    ldr     sp, =irq_stack_top
    /* TODO ... */
#endif

    /* Enter the kernel main */
    ldr     r12, =kmain
    mov     lr, pc      /* set the return address */
    bx      r12         /* the target code can be ARM or THUMB */

    swi     0xFFFFFF    /* cause exception if main() ever returns */

.global freeze
.type freeze, %function
freeze:
    /* TODO disable interrupts */
    /* cpu does not support suport wfi while in ARM mode */
    b       freeze /* just in case... */
.size freeze, .-freeze


.section .bss

.global kstack
kstack:
.space KSTACK_SIZE
