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

#include <signal.h>
#include <unistd.h>

static void restorer(void)
{
    syscall(__NR_sigreturn);
}

int sigaction(int sig, const struct sigaction *act,
        struct sigaction *oact)
{
    struct sigaction iact = *act;

    iact.sa_restorer = restorer;
    return syscall(__NR_sigaction, sig, &iact, oact);
}
