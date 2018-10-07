/*
 * Copyright (c) 2015-2018, Davide Galassi. All rights reserved.
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

#ifndef BEEOS_ARCH_X86_ISR_ARCH_H_
#define BEEOS_ARCH_X86_ISR_ARCH_H_

#include <stdint.h>

/*
 * Architecture-specific interrupt stack frame.
 * Some registers are automatically pushed by the processor while
 * others are pushed by the dispatching code for preservation or
 * for later reference.
 */
struct isr_frame {
    uint32_t    ds;         /* data segment selector */
    uint32_t    edi;        /* pushed by pusha */
    uint32_t    esi;        /* pushed by pusha */
    uint32_t    ebp;        /* pushed by pusha */
    uint32_t    esp;        /* pushed by pusha */
    uint32_t    ebx;        /* pushed by pusha */
    uint32_t    edx;        /* pushed by pusha */
    uint32_t    ecx;        /* pushed by pusha */
    uint32_t    eax;        /* pushed by pusha */
    uint32_t    int_no;     /* interrupt number */
    uint32_t    err_no;     /* error code */
    uint32_t    eip;        /* pushed by the processor */
    uint32_t    cs;         /* pushed by the processor */
    uint32_t    eflags;     /* pushed by the processor */
    uint32_t    usr_esp;    /* pushed by the processor */
    uint32_t    ss;         /* pushed by the processor */
};

/*
 * Interrupt numbers
 */
#define ISR_DIV_BY_ZERO 0
#define ISR_PAGE_FAULT  14
#define ISR_IRQ0        32
#define ISR_TIMER       32
#define ISR_KEYBOARD    33
#define ISR_COM2        35
#define ISR_COM1        36
#define ISR_SYSCALL     128

#endif /* BEEOS_ARCH_X86_ISR_ARCH_H_ */
