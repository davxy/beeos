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

#include "kprintf.h"
#include "proc.h"
#include "timer.h"
#include "kmalloc.h"
#include "sys.h"

struct task ktask;
struct task *current_task;
struct task *ready_queue;


int sigpop(sigset_t *sigpend, sigset_t *sigmask)
{
    int sig;
    /* find first non blocked signal */
    for (sig = 0; sig < SIGNALS_NUM; sig++)
        if (sigismember(sigpend, sig) == 1 && sigismember(sigmask, sig) <= 0)
            break;
    if (sig == SIGNALS_NUM)
        return -1;
    sigdelset(sigpend, sig);
    return sig;
}

int do_signal(void)
{
    struct sigaction *act;
    int sig;
    struct isr_frame *ifr;
    uint32_t *esp;

    sig = sigpop(&current_task->sigpend, &current_task->sigmask);
    if (sig < 0)
        return -1; /* no unmasked signals available */
    ifr = current_task->arch.ifr;
    act = &current_task->signals[sig-1];

    if (act->sa_handler == SIG_DFL)
    {
        if (sig == SIGCHLD || sig == SIGURG)
            return 0; /* Ignore */
        else if (sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU)
            return 0; /* TODO: Stop the process */
        else
            sys_exit(1); /* Terminate the process */
    }

    if (!act->sa_restorer || act->sa_handler == SIG_IGN)
        return 0; /* No way to return from signal handlers or ignore */

    if (!current_task->arch.sfr)
    {
        current_task->arch.sfr = kmalloc(sizeof(*ifr), 0);
        if (!current_task->arch.sfr)
        {
            kprintf("[warn] no memory to handle signal (%d)\n", sig);
            return 0;
        }
        memcpy(current_task->arch.sfr, ifr, sizeof(*ifr));
    }

    /* Adjust user stack to return in the signal handler */
    esp = (uint32_t *)ifr->usr_esp;
    *--esp = sig;   // signum
    *--esp = (uint32_t)act->sa_restorer;
    ifr->usr_esp = (uint32_t)esp;
    ifr->eip = (uint32_t)act->sa_handler;

    return 0;
}

void scheduler(void)
{
    struct task *curr;
    struct task *next;
    
    curr = current_task;
    next = list_container(current_task->tasks.next, 
            struct task, tasks);

    while (next->state != TASK_RUNNING && next != current_task)
        next = list_container(next->tasks.next, struct task, tasks);
    
    if (next == current_task && next->pid != 0)
    {
        /* Nothing to run... run the idle() task */
        ktask.state = TASK_RUNNING;
        next = &ktask;
    }

    current_task = next;
    task_arch_switch(&curr->arch, &next->arch);

    current_task->counter = msecs_to_ticks(SCHED_TIMESLICE);
}

void scheduler_init(void)
{
    int i;

    /* Set to zero: uids, gids, pids... */
    memset(&ktask, 0, sizeof(ktask));
    ktask.cwd = NULL;
    ktask.state = TASK_RUNNING;
    ktask.brk = 0;
    list_init(&ktask.tasks);
    list_init(&ktask.sibling);
    list_init(&ktask.children);
    list_init(&ktask.condw);
    list_init(&ktask.timers);
    task_arch_init(&ktask.arch);

    (void)sigemptyset(&ktask.sigmask);
    (void)sigemptyset(&ktask.sigpend);
    for (i = 0; i < SIGNALS_NUM; i++)
    {
        memset(&ktask.signals[i], 0, sizeof(struct sigaction));
        ktask.signals[i].sa_handler = SIG_DFL;
    }

    current_task = &ktask;
}

void task_dump(struct task *t)
{
    char state;

    switch (t->state)
    {
        case TASK_RUNNING:
            state = 'R';
            break;
        case TASK_SLEEPING:
            state = 'S';
            break;
        default:
            state = 'U';
            break;
    }
    kprintf("<pid=%d, ppid=%d, pgid=%d, state=%c)>",
              t->pid, t->pptr->pid, t->pgid, state);
}


void proc_dump_p(struct task *t, int level,
        struct task *fs, struct task *fp)
{
    int i;
    struct task *s;

    for (i = 0; i < level; i++)
        kprintf(" ");
    task_dump(t);
    kprintf("\n");
    s = struct_ptr(t->sibling.next, struct task, sibling);
    if (s != fs) {
        proc_dump_p(s, level, fs, s);
    }
    t = struct_ptr(t->children.next, struct task, children);
    if (t != fp)
        proc_dump_p(t, level + 1, t, fp);
}

void proc_dump(void)
{
    proc_dump_p(&ktask, 0, &ktask, &ktask);
}
