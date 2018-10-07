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

int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    int sig;
    int res = 0;

    if (oset != NULL)
        memcpy(oset, &current->sigmask, sizeof(sigset_t));

    if (set != NULL) {
        if (how == SIG_SETMASK) {
            memcpy(&current->sigmask, set, sizeof(sigset_t));
        } else {
            for (sig = 0; sig < SIGNALS_NUM; sig++) {
                if (sigismember(set, sig) > 0) {
                    if (how == SIG_BLOCK) {
                        sigaddset(&current->sigmask, sig);
                    } else if (how == SIG_UNBLOCK) {
                        sigdelset(&current->sigmask, sig);
                    } else {
                        res = -EINVAL;
                        break;
                    }
                }
            }
        }
    }
    return res;
}
