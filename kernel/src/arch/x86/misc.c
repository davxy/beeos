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
#include "arch/x86/isr_arch.h"
#include <signal.h>


void sigret_prepare(struct isr_frame *ifr,
                    const struct sigaction *act, int sig)
{
    uint32_t *esp;

    /* Adjust user stack to return in the signal handler */
    esp = (uint32_t *)ifr->usr_esp;
    *(--esp) = sig;
    *(--esp) = (uint32_t)act->sa_restorer;
    ifr->usr_esp = (uint32_t)esp;
    ifr->eip = (uint32_t)act->sa_handler;
}
