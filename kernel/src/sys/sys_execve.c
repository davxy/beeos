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
#include "fs/vfs.h"
#include "elf.h"
#include "kmalloc.h"
#include "kprintf.h"
#include "proc.h"
#include "arch/x86/paging.h"
#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>


static char *push(char *sp, const char *str)
{
    size_t n;

    n = strlen(str) + 1;
    sp -= n;
    memcpy(sp, str, n);
    return sp;
}

static char *push_all(uintptr_t *base, char *sp, const char * const str[],
                      ptrdiff_t delta, uintptr_t *nout)
{
    int n;

    for (n = 0; str[n] != NULL; n++)
        ;
    if (nout != NULL)
        *nout = n;
    base[n] = 0;
    while (n-- > 0) {
        sp = push(sp, str[n]);
        base[n] = (uintptr_t)sp + delta;
    }
    return sp;
}

/*
 * Populates the user-space stack with a valid argv and envp pointers.
 * The stack requires the final user-stack addresses but actually the structure
 * is constructed using a chunk of memory allocated in kernel space (via
 * kmalloc). Thus every address stored in the user-stack is adjusted using
 * a 'delta' value. This delta is the difference between the top of the
 * kernel-resident version of the stack  and the top of the user space version
 * (that is set equal to the constant KVBASE).
 *
 * Stack bottom elements to be populated:
 *
 *       [  NULL   ]   end of envp marker
 *       [  ....   ]
 *  +--> [ envp[0] ]
 *  |    [  NULL   ]   end of argv marker
 *  |    [  ...... ]
 *  |    [ argv[0] ] <--+
 *  +--- [ **envp  ]    |
 *       [ **argv  ] ---+
 *       [   argc  ]
 *
 * Note: passed argv and envp strings are copied on the top of the stack.
*/
static void stack_init(uintptr_t *base, const char * const argv[],
                       const char * const envp[])
{
    char *sp = (char *)base + ARG_MAX;
    ptrdiff_t delta = (char *)KVBASE - sp;

    sp = push_all(&base[3], sp, argv, delta, &base[0]);
    base[1] = (uintptr_t)&base[3] + delta;

    if (envp != NULL)
        sp = push_all(&base[4 + base[0]], sp, envp, delta, NULL);
    else
        base[4 + base[0]] = 0;  /* Empty environment array */

    base[2] = (uintptr_t)&base[4+base[0]] + delta;
}

static int segment_init(const struct elf_prog_hdr *ph, struct inode *inod)
{
    int ret = 0;
    uint32_t vaddr;

    if (ph->memsz < ph->filesz || KVBASE <= ph->vaddr + ph->memsz)
        return -ENOEXEC;

    /* Look for program brk (temporary... not very elegant) */
    if ((ph->flags & ELF_PROG_FLAG_READ) != 0 &&
        (ph->flags & ELF_PROG_FLAG_WRITE) != 0 &&
        current->brk < ph->vaddr + ph->memsz) {
        current->brk = ph->vaddr + ph->memsz;
    }

    for (vaddr = ALIGN_DOWN(ph->vaddr, PAGE_SIZE);
         vaddr < ph->vaddr + ph->memsz;
         vaddr += PAGE_SIZE) {
        if ((ret = (int)page_map((char *)vaddr, -1)) < 0)
            return ret;
    }

    if (ph->filesz != 0) {
        ret = vfs_read(inod, (void *)ph->vaddr, ph->filesz, ph->offset);
        if (ret != ph->filesz) {
            if (ret >= 0)
                ret = -EIO;
            return ret;
        }
    }

    if (ph->memsz - ph->filesz > 0)
        memset((void *)(ph->vaddr + ph->filesz), 0, ph->memsz - ph->filesz);

    return ret;
}


int sys_execve(const char *path, const char *const argv[],
               const char *const envp[])
{
    int ret = 0;
    struct elf_hdr eh;
    struct elf_prog_hdr ph;
    struct dentry *dent;
    struct inode *inod;
    unsigned int i, off;
    uint32_t pgdir;
    void *ustack;

    if (current->arch.ifr == NULL || argv == NULL)
        return -EINVAL;

    dent = named(path);
    if (dent == NULL)
        return -ENOENT;
    inod = dent->inod;

    if (vfs_read(inod, &eh, sizeof(eh), 0) != sizeof(eh) ||
            eh.magic != ELF_MAGIC) {
        dput(dent);
        return -ENOEXEC;
    }

    /*
     * Immediatelly copy argv and envp arrays in a temporary user stack
     * allocated via kmalloc (shared betweek virtual spaces).
     */
    ustack = kmalloc(ARG_MAX, 0);
    if (!ustack) {
        dput(dent);
        return -ENOMEM;
    }
    stack_init((uintptr_t *)ustack, argv, envp);

    pgdir = page_dir_dup(0);
    page_dir_switch(pgdir);

    /* The function has been called via a syscall */
    /* TODO: Create user stack only if we where in user space
     * Otherwise esp is not in the frame */
    /* Minimal user stack */
    if ((ret = (int)page_map((char *)KVBASE-PAGE_SIZE, -1)) < 0)
        goto bad;
    memcpy((char *)KVBASE-ARG_MAX, ustack, ARG_MAX);

    /* Release user stack copy */
    kfree(ustack, ARG_MAX);

    /* Start with an unknown program break */
    current->brk = 0;

    off = eh.phoff;
    for (i = 0, off = eh.phoff; i < eh.phnum; i++) {
        ret = vfs_read(inod, &ph, sizeof(ph), off);
        if (ret != sizeof(ph)) {
            if (ret >= 0)
                ret = -EIO;
            goto bad;
        }

        if (ph.type == ELF_PROG_TYPE_LOAD) {
            ret = segment_init(&ph, inod);
            if (ret < 0)
                goto bad;
        }

        off += sizeof(struct elf_prog_hdr);
    }

    /*** FIXME ARCH specific code ***/

    /* Release the old dir just before jump */
    page_dir_del(current->arch.pgdir);
    current->arch.pgdir = pgdir;

    /* We assume that ARG_MAX is lass than PAGE_SIZE */
    current->arch.ifr->usr_esp = KVBASE-ARG_MAX;
    current->arch.ifr->eip = eh.entry;

    /*
     * Eventually close files with O_CLOEXEC flag enabled
     */
    for (i = 0; i < OPEN_MAX; i++) {
        if (current->fds[i].fil != NULL &&
           (current->fds[i].flags & O_CLOEXEC) != 0) {
            if (sys_close(i) < 0)
                kprintf("[warn] error closing an open file\n");
        }
    }

    /*
     * POSIX1. All signals are set to their default action unless the process
     * that calls exec is ignoring the signal.
     */
    for (i = 0; i < SIGNALS_NUM; i++) {
        if (current->signals[i].sa_handler != SIG_IGN) {
            memset(&current->signals[i], 0, sizeof(struct sigaction));
            current->signals[i].sa_handler = SIG_DFL;
        }
    }

    dput(dent);
    return ret;

bad:
    dput(dent);
    kfree(ustack, ARG_MAX);
    /* Switch back to the old dir */
    page_dir_switch(current->arch.pgdir);
    /* Release the new dir, this also release all the mapped pages. */
    page_dir_del(pgdir);
    return ret;
}
