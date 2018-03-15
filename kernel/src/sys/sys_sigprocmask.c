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

#include "sys.h"
#include "proc.h"
#include <errno.h>

int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    struct task *cur = current;

    if (oset != NULL)
        memcpy(oset, &cur->sigmask, sizeof(sigset_t));

    if (set != NULL)
    {
        if (how == SIG_SETMASK)
            memcpy(&cur->sigmask, set, sizeof(sigset_t));
        else
        {
            int sig;
            for (sig = 0; sig < SIGNALS_NUM; sig++) {
                if (sigismember(set, sig) > 0) {
                    if (how == SIG_BLOCK)
                        sigaddset(&cur->sigmask, sig);
                    else if (how == SIG_UNBLOCK)
                        sigdelset(&cur->sigmask, sig);
                    else
                        return -EINVAL;
                }
            }
        }
    }

    return 0;
}
