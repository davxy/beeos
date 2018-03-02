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


struct tss_struct tss;

void swtch(struct context **old, struct context *new);


/*
 * TODO : implement as clone syscall
 */
int task_arch_init(struct task_arch *task, task_entry_t entry)
{
    char *ti;
    uint32_t *sp;

    task->ifr = NULL;
    task->sfr = NULL;

    if (task == &ktask.arch)
    {
        /* The task 0 does not need complete initialization */
        task->pgdir = (uint32_t)virt_to_phys(kpage_dir);
        task->ctx = NULL;
    }

    task->pgdir = page_dir_dup(1);
    if ((int)task->pgdir < 0)
        return (int)task->pgdir; /* Fail */

    /* Stack creation */
    ti = kmalloc(KSTACK_SIZE, 0);
    if (ti == NULL)
        return -1;
    sp = (uint32_t *)(ti + KSTACK_SIZE);

    if (current_task->arch.ifr != NULL)
    {
        struct isr_frame *ifr2 = ((struct isr_frame *)sp) - 1;

        sp = (uint32_t *)ifr2;
        /* child ifr is equal to the parent but fork returns 0 in the child */
        *ifr2 = *current_task->arch.ifr;
        ifr2->eax = 0;
    }

    task->ctx = ((struct context *)sp) - 1;
    task->ctx->ebp = (uint32_t)sp;
    task->ctx->eip = (uint32_t)entry;
    task->ctx->ebx = 0;
    task->ctx->edi = 0;
    task->ctx->esi = 0;

    return 0;
}

void task_arch_deinit(struct task_arch *task)
{
    kfree((void *)ALIGN_DOWN((uint32_t)task->ctx, KSTACK_SIZE), KSTACK_SIZE);
    page_dir_del(task->pgdir);
}

void task_arch_switch(struct task_arch *curr, struct task_arch *next)
{
    tss.esp0 = ALIGN_UP((uint32_t)next->ctx, KSTACK_SIZE);
    page_dir_switch(next->pgdir);

    /* Execute this as the last statement. Can throw us in another place */
    swtch(&curr->ctx, next->ctx);
}

