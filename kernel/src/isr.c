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

#include "isr.h"
#include "sys.h"
#include "proc.h"
#include "panic.h"
#include "kprintf.h"

#include "arch/x86/pic.h"


int need_resched;

#define HANDLERS_NUM    49
static isr_handler_t isr_handlers[HANDLERS_NUM];


/* ISR arch independent dispatcher */
void isr_handler(struct isr_frame *ifr)
{
    struct isr_frame *previfr;
    unsigned int num;

    /*
     * Save the current ifr pointer in the stack. This allows nested
     * interrupts handling (e.g. manage a page fault in the kernel).
     * Thus if, after isr handling, we return back to the kernel
     * we have the correct 'ifr' value within the current_task struct.
     */
    previfr = current->arch.ifr;
    current->arch.ifr = ifr;

    num = ifr->int_no;
    if (num == ISR_SYSCALL)
        num = 48;

    if (num >= HANDLERS_NUM || isr_handlers[num] == NULL)
        panic("unhandled interrupt %d\n", num);

    isr_handlers[num]();

    /* For IRQs send EOI to the PIC */
    if (32 <= num && num <= 47)
        pic_eoi(num);

    if (need_resched != 0) {
        need_resched = 0;
        scheduler();
    }

    /*
     * Process pending signals queue.
     * Do not handle nested signals (sfr should be null) and
     * handle signals only before return to user code (CS check).
     */
    if (!sigisemptyset(&current->sigpend) &&
            current->arch.sfr == NULL && (ifr->cs & 0x3) == 0x3)
        do_signal();

    /* Eventually restore the previous ifr */
    current->arch.ifr = previfr;
}

/*
 * Registers an interrupt handler
 */
void isr_register_handler(unsigned int num, isr_handler_t func)
{
    if (num == ISR_SYSCALL)
        num = 48;
    if (num < HANDLERS_NUM) {
        isr_handlers[num] = func;
        /* TODO: the following is ARCH specific code */
        if (32 <= num && num <= 47)
            pic_unmask(num - 32);
    } else {
        panic("error: isr num (%d) out of range\n", num);
    }
}


static void divide_error(void)
{
    task_signal(current, SIGFPE);
}

static void invalid_opcode(void)
{
    task_signal(current, SIGILL);
}

void isr_init(void)
{
    /* Register core traps routines */
    isr_register_handler(0, divide_error);
    isr_register_handler(6, invalid_opcode);
}
