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

#ifndef BEEOS_ARCH_X86_TASK_H_
#define BEEOS_ARCH_X86_TASK_H_

#include "paging.h"
#include "isr.h"

#define KSTACK_SIZE     PAGE_SIZE

/** Kernel process 0 page directory */
extern uint32_t kpage_dir[1024];

/** Kernel process 0 stack. */
extern uint8_t kstack[KSTACK_SIZE];

/*
 * We just need two entries that defines the stack pointer and the stack
 * segment when we switch to kernel mode. All the other entries are unused
 */
struct tss_struct {
    uint32_t prev;
    uint32_t esp0;  /* Stack pointer when we change to kernel mode */
    uint32_t ss0;   /* Stack segment when we change to kernel mode */
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
};

extern struct tss_struct    tss;


/**
 * Saved registers for kernel context switches.
 *
 * Don't need to save all the segment registers (%cs, etc), because they are
 * constant across kernel contexts.
 * Don't need to save eax, ecx, edx, because the x86 convention is that the
 * caller has saved them.
 * Contexts are stored at the bottom of the stack they describe; the stack
 * pointer is the address of the context.
 * The layout of the context matches the layout of the stack in switch.S
 * at the "Switch stacks" comment. Switch doesn't save eip explicitly,
 * but it is on the stack and task_alloc() manipulates it.
 */
struct context {
    uint32_t    edi;
    uint32_t    esi;
    uint32_t    ebx;
    uint32_t    ebp;
    uint32_t    eip;
};

struct task_arch {
    uint32_t            pgdir;  /**< Page directory physical address */
    struct context      *ctx;   /**< Task state context */
    struct isr_frame    *ifr;   /**< Interrupt frame */
    struct isr_frame    *sfr;   /**< Interrupt saved frame (used by signals) */
};

#endif /* BEEOS_ARCH_X86_TASK_H_ */
