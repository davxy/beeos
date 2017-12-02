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

#include "proc/task.h"
#include "paging.h"
#include "kmalloc.h"
#include <stddef.h>

extern uint32_t get_eip();

extern struct task ktask;
extern struct task *current_task;

#define KSTACKSIZE  4096 

int task_arch_init(struct task_arch *task)
{
    if (task == &ktask.arch)
    {
        task->pgdir = (uint32_t)virt_to_phys(kpage_dir);
        return 0;
    }

    task->pgdir = page_dir_dup(0);
    if ((int)task->pgdir < 0)
        return (int)task->pgdir;

    task->ifr = NULL;
    task->sfr = NULL;

    asm volatile("mov   %0, esp \n\t"
                 "mov   %1, ebp \n\t"
                : "=r"(task->esp),
                  "=r"(task->ebp));
    task->eip = get_eip();

    return 0;
}

void task_arch_deinit(struct task_arch *task)
{
    page_dir_del(task->pgdir, 0);
}

void task_arch_switch(struct task_arch *curr, struct task_arch *next)
{
    asm volatile("mov   %0, esp \n\t"
                 "mov   %1, ebp \n\t"
                 "mov   %2, offset switch_end \n\t"
                : "=r"(curr->esp),
                  "=r"(curr->ebp),
                  "=r"(curr->eip));

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
