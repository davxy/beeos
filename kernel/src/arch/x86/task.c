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

#include "proc.h"
#include "arch/x86/task.h"
#include "paging.h"
#include "kmalloc.h"
#include <stddef.h>


extern struct task ktask;
extern struct task *current_task;

/*
 * TODO : implement as clone syscall
 */
int task_arch_init(struct task_arch *task)
{
    char *ti;
    extern uint32_t fork_ret;

    if (task == &ktask.arch)
    {
        /* The task 0 does not need complete initialization */
        task->pgdir = (uint32_t)virt_to_phys(kpage_dir);
        return 0;
    }

    task->pgdir = page_dir_dup(1);
    if ((int)task->pgdir < 0)
        return (int)task->pgdir; /* Fail */

    task->ifr = NULL;
    task->sfr = NULL;

    ti = kmalloc(KSTACK_SIZE, 0);
    if (ti == NULL)
        return -1;

    task->ebp = (uint32_t)ti + KSTACK_SIZE;
    task->esp = task->ebp;
    task->eip = (uint32_t)&fork_ret;

    if (current_task->arch.ifr != NULL)
    {
        struct isr_frame *ifr2;
        
        task->esp -= sizeof(struct isr_frame);
        ifr2 = (struct isr_frame *)task->esp;
        *ifr2 = *current_task->arch.ifr;
        ifr2->eax = 0; /* fork returns 0 in the child */
    }
    return 0;
}

void task_arch_deinit(struct task_arch *task)
{
    kfree((void *)ALIGN_DOWN(task->esp, KSTACK_SIZE), KSTACK_SIZE);
    page_dir_del(task->pgdir);
}

struct tss_hdr {
    uint32_t next;
    uint32_t esp0;
};

void task_arch_switch(struct task_arch *curr, struct task_arch *next)
{
    extern struct tss_hdr tss;

    asm volatile("mov   %0, esp \n\t"
                 "mov   %1, ebp \n\t"
                 "mov   %2, offset switch_end \n\t"
                : "=r"(curr->esp),
                  "=r"(curr->ebp),
                  "=r"(curr->eip));

    tss.esp0 = ALIGN_UP(next->esp, KSTACK_SIZE);

    asm volatile("mov    esp, %0 \n\t"
                 "mov    ebp, %1 \n\t"
                 "mov    cr3, %2 \n\t"
                 "jmp    %3      \n\t"
                 "switch_end:    \n\t"
              : : "r"(next->esp), 
                  "r"(next->ebp), 
                  "r"(next->pgdir),
                  "r"(next->eip));
}

