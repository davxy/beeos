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
#include "misc.h"

/*
 * Kernel idle procedure.
 * This endless procedure is executed by the first kernel process when
 * there is nothing useful to do.
 */
void idle(void)
{
    do {
        current->state = TASK_SLEEPING;
        scheduler();
        sti(); /* Enable interrupts */
        hlt(); /* ...before halt the processor */
        cli(); /* Disable interrupts in kernel code */
    } while (current->state == TASK_RUNNING);
}
