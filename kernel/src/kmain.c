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

#include "kprintf.h"
#include "kmalloc.h"
#include "version.h"
#include "panic.h"
#include "timer.h"
#include "sys.h"
#include "proc.h"
#include "mm/slab.h"
#include "driver/tty.h"
#include "fs/vfs.h"
#include "fs/devfs/devfs.h"
#include "proc/task.h"
#include "dev.h"


#define ROOT_FS_TYPE    "ext2"
#define ROOT_DEV        DEV_INITRD

/* Init process entry point (arch defined) */
void init(void);


static void mount_root(void)
{
    const struct super_block *sb;

    /*
     * DEVFS is temporary mounted as system root.
     * To create the initrd node (used to read the real fs from the disk.
     */
    sb = vfs_super_create(0, "dev");
    if (sb == NULL)
        panic("Unable to create dev fs");

    current->cwd = ddup(sb->root);
    current->root = ddup(sb->root);

    /* Initrd node created to read data from ramdisk */
    if (sys_mknod("/initrd", S_IFBLK, DEV_INITRD) < 0)
        panic("Creating initrd temporary root");

    /*
     * Initialization finished
     */

    sb = vfs_super_create(ROOT_DEV, ROOT_FS_TYPE);
    if (sb == NULL)
        panic("Unable to create root file system");

    dput(current->root);
    dput(current->cwd);
    current->root = ddup(sb->root);
    current->cwd  = ddup(sb->root);
}

void arch_final(void);

void kmain(void)
{
    /* Core */
    slab_init();
    kmalloc_init();
    isr_init();

    /* Primary */
    timer_init();
    vfs_init();
    scheduler_init();
    tty_init();
    syscall_init();

    /* Finish machine specific initialization */
    arch_final();

    /* Mount root filesystem */
    mount_root();

    kprintf("BeeOS v%d.%d.%d - %s\n\n",
            BEEOS_MAJOR, BEEOS_MINOR, BEEOS_PATCH, BEEOS_CODENAME);

    /* Start the init process */
    if (task_create(init) == NULL)
        panic("Unable to start init task");

    /* Process 0 continues with the idle procedure */
    idle();
}
