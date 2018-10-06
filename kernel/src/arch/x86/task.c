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
int task_arch_init(struct task_arch *tsk, task_entry_t entry)
{
    char *ti;
    uint32_t *sp;

    tsk->ifr = NULL;
    tsk->sfr = NULL;

    if (tsk == &ktask.arch) {
        /* The task 0 does not need complete initialization */
        tsk->pgdir = (uint32_t)virt_to_phys(kpage_dir);
        tsk->ctx = NULL;
    }

    tsk->pgdir = page_dir_dup(1);
    if ((int)tsk->pgdir < 0)
        return (int)tsk->pgdir; /* Fail */

    /* Stack creation */
    ti = (char *)kmalloc(KSTACK_SIZE, 0);
    if (ti == NULL)
        return -1;

    sp = (uint32_t *)ALIGN_DOWN((uintptr_t)ti + KSTACK_SIZE, sizeof(uint32_t));

    if (current->arch.ifr != NULL) {
        struct isr_frame *ifr2 = ((struct isr_frame *)sp) - 1;

        sp = (uint32_t *)ifr2; /* Safe... ifr2 should be aligned to uint32_t */
        /* Child ifr is equal to the parent but fork returns 0 in the child */
        *ifr2 = *current->arch.ifr;
        ifr2->eax = 0;
    }

    tsk->ctx = ((struct context *)sp) - 1;
    tsk->ctx->ebp = (uint32_t)sp;
    tsk->ctx->eip = (uint32_t)entry;
    tsk->ctx->ebx = 0;
    tsk->ctx->edi = 0;
    tsk->ctx->esi = 0;

    return 0;
}

void task_arch_deinit(struct task_arch *tsk)
{
    kfree((void *)ALIGN_DOWN((uint32_t)tsk->ctx, KSTACK_SIZE), KSTACK_SIZE);
    page_dir_del(tsk->pgdir);
}

void task_arch_switch(struct task_arch *curr, const struct task_arch *next)
{
    tss.esp0 = ALIGN_UP((uint32_t)next->ctx, KSTACK_SIZE);
    page_dir_switch(next->pgdir);

    /* Execute this as the last statement. Can throw us in another place */
    swtch(&curr->ctx, next->ctx);
}
