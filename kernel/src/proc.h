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

#ifndef BEEOS_PROC_H_
#define BEEOS_PROC_H_

#include "proc/task.h"

/* Default process timeslice (milliseconds) */
#define SCHED_TIMESLICE     100

extern struct task *current;
extern struct task ktask;

extern int need_resched;


void scheduler(void);

void scheduler_init(void);

/**
 * Process pending (non masked) signals.
 */
int do_signal(void);

/*
 * Arch dependent return preparation from a signal handler.
 */
void sigret_prepare(struct isr_frame *ifr,
                    const struct sigaction *act, int sig);


/*
 * Start init user-mode process.
 */
void init_start(void);

/**
 * Idle procedure.
 * This is executed by the first kernel process (pid=0) after that the
 * init process (pid=1) has been started.
 */
void idle(void);


void proc_dump(void);


#endif /* BEEOS_PROC_H_ */
