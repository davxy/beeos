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

#include "sys.h"
#include "proc.h"
#include <errno.h>

int sys_sigsuspend(const sigset_t *mask)
{
    sigset_t omask;

    if (mask == NULL)
        return -EFAULT;

    /* Set the new mask saving the old one */
    memcpy(&omask, &current->sigmask, sizeof(sigset_t));
    memcpy(&current->sigmask, mask, sizeof(sigset_t));

    current->arch.ifr->eax = -EINTR;
    /*
     * Before re-establish the old mask we must first eventually process
     * pending signals using the current mask
     */
    while (do_signal() < 0) {
        current->state = TASK_SLEEPING;
        scheduler(); /* Release the CPU */
    }

    /* Restore old mask */
    memcpy(&current->sigmask, &omask, sizeof(sigset_t));
    return 0;
}
