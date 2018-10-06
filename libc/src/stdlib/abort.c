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

#include <stdlib.h>
#include <signal.h>

void abort(void)
{
    sighandler_t prev;

    while (1) {
        prev = signal(SIGABRT, SIG_DFL);
        if (prev != SIG_DFL && prev != SIG_IGN && prev != SIG_ERR)
            signal(SIGABRT, prev);
        raise(SIGABRT);
    }
}
