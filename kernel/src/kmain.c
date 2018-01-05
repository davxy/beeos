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
#include "kmalloc.h"
#include "version.h"
#include "panic.h"
#include "timer.h"
#include "sys.h"
#include "proc.h"
#include "driver/tty.h"
#include "fs/vfs.h"
#include "proc/task.h"
#include "dev.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define ROOT_FS_TYPE    "ext2"
#define ROOT_DEV        DEV_INITRD

void kmain(void)
{
    struct sb *sb;

    kprintf("BeeOS v%d.%d.%d - %s\n\n", 
            BEEOS_MAJOR, BEEOS_MINOR, BEEOS_PATCH, BEEOS_CODENAME);

    /*
     * Core
     */
    
    kmalloc_init();
    isr_init();

    /*
     * Primary
     */

    timer_init(100);
    fs_init();
    scheduler_init();
    tty_init();
    syscall_init();

    /*
     * Initialization finished
     */

    kprintf("mounting root fs\n");
    sb = vfs_sb_create(ROOT_DEV, ROOT_FS_TYPE);
    if (sb == NULL)
        panic("unable to mount root file system");
    current_task->cwd = sb->root;

    /*
     * Fork and start the init process
     */

#if 0
    if (sys_fork() == 0)
    {
        init();
        panic("init returned");
    }
#endif
    void init_start(void);
    init_start();

    /*
     * Idle procedure
     */

    idle();
}

