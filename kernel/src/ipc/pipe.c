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

#include "sync/spinlock.h"
#include "fs/vfs.h"
#include "proc.h"
#include "sync/cond.h"
#include "kmalloc.h"
#include "sys.h"
#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

#define PIPE_SIZE   PIPE_BUF
#define DATA_SIZE   (PIPE_SIZE+1)

struct file *fs_file_alloc(void);

/* Implemented as a ring-buffer */
struct pipe_inode
{
    struct inode base;
    struct cond  queue;
    int nrp;                /**< Next read position */
    int nwp;                /**< Next write position */
    int queued_writers;     /**< Number of stopped writers */
    int queued_readers;     /**< Number of queued readers */
    char data[DATA_SIZE];   /**< Pipe data */
};

/* TODO: in VFS this is a 'file' operation. 
 * Thus the function should take a file and as a consequence 
 * we can check if that is not blocking */

/*
 * From APUE
 * If we read from a pipe whose write end has been closed, read returns 0 to indicate an end of file after
 * all the data has been read. (Technically, we should say that this end of file is not generated until there are
 * no more writers for the pipe. It's possible to duplicate a pipe descriptor so that multiple processes have
 * the pipe open for writing. Normally, however, there is a single reader and a single writer for a pipe.
 */
static int pipe_read(struct inode *inode, void *buf,
        size_t count, off_t offset)
{
    int n, left;
    char *ptr = (char *)buf;
    struct pipe_inode *pnode = (struct pipe_inode *)inode;

    left = count;
    spinlock_lock(&pnode->queue.lock);
    while (left > 0) {
        while (pnode->nrp == pnode->nwp) { /* empty buffer */
            /*
             * WARNING: in case of multiple writers this condition never
             * holds and there is deadlock risk.
             */
            if (pnode->base.ref == 1) {
                spinlock_unlock(&pnode->queue.lock);
                return 0;
            }

            /* If something has been read and there are no queued writers */
            if (left != count && pnode->queued_writers == 0)
                goto done;

            /* TODO: if BLOCKING allowed */ 
            pnode->queued_readers++;
            if (pnode->queued_writers > 0)      /* if there are pending writers */
                cond_broadcast(&pnode->queue);  /* wakeup them before sleep */
            cond_wait(&pnode->queue);
            pnode->queued_readers--;
        }

        if (pnode->nrp < pnode->nwp)
            n = MIN(left, pnode->nwp - pnode->nrp);
        else
            n = MIN(left, DATA_SIZE - pnode->nrp);

        memcpy(ptr, &pnode->data[pnode->nrp], n);
        ptr += n;
        pnode->nrp += n;
        if (pnode->nrp == DATA_SIZE)
            pnode->nrp = 0;
        left -= n;
    }
done:
    spinlock_unlock(&pnode->queue.lock);
    n = count-left;
    /* Notify if something has been read or a writer MUST be woken up */
    if (n > 0 || (pnode->base.ref == 1 && pnode->queued_writers > 0))
        cond_broadcast(&pnode->queue);
    return n;
}

/*
 * From APUE.
 * If we write to a pipe whose read end has been closed, the signal SIGPIPE is generated. If we either
 * ignore the signal or catch it and return from the signal handler, write returns –1 with errno set to EPIPE .
 */
static int pipe_write(struct inode *inode, const void *buf,
        size_t count, off_t offset)
{
    size_t n, left;
    char *ptr = (char *)buf;
    struct pipe_inode *pnode = (struct pipe_inode *)inode;

    left = count;
    spinlock_lock(&pnode->queue.lock);
    while (left > 0)
    {
        /* Check if full */
        while (pnode->nwp+1 == pnode->nrp
         || (pnode->nwp+1 == DATA_SIZE && pnode->nrp == 0))
        {
            /*
             * No more readers.
             * WARNING: in case of multiple writers this condition never
             * holds and there is deadlock risk.
             */
            if (pnode->base.ref == 1)
            {
                spinlock_unlock(&pnode->queue.lock);
                sys_kill(sys_getpid(), SIGPIPE);
                /* in case the signal has been catched, return an error */
                return -EPIPE;
            }

            //if (BLOKING)
            pnode->queued_writers++;
            if (pnode->queued_readers > 0) /* there are pending writers */
                cond_broadcast(&pnode->queue); /* wakeup all before (eventually) sleep */
            cond_wait(&pnode->queue);
            pnode->queued_writers--;

            //else // unlock first!!!
            //    return goto spinlock unlock;
        }

        if (pnode->nrp <= pnode->nwp)
        {
            n = MIN(left, DATA_SIZE - pnode->nwp);
            if (pnode->nwp + n == DATA_SIZE && pnode->nrp == 0)
                n--;
        }
        else
            n = MIN(left, pnode->nrp - pnode->nwp - 1);

        memcpy(&pnode->data[pnode->nwp], ptr, n);
        ptr += n;
        pnode->nwp += n;
        if (pnode->nwp == DATA_SIZE)
            pnode->nwp = 0;
        left -= n;
    }
    spinlock_unlock(&pnode->queue.lock);
    n = count - left;
    /* Notify if something has been written or a reader MUST be woken up */
    if (n > 0 || (pnode->base.ref == 1 && pnode->queued_readers > 0))
        cond_broadcast(&pnode->queue);
    return n;
}

static const struct inode_ops pipe_ops =
{
    .read = pipe_read,
    .write = pipe_write
};

struct inode *pipe_inode_create(void)
{
    struct pipe_inode *pnode;
    pnode = kmalloc(sizeof(struct pipe_inode), 0);
    if (!pnode)
        return NULL;
    memset(pnode, 0, sizeof(*pnode));
    pnode->base.mode = S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO;
    pnode->base.ops = &pipe_ops;
    pnode->base.ref = 2;
    cond_init(&pnode->queue);
    return &pnode->base;
}

/* TODO : all error checking and rollback code is missing */

int pipe_create(int pipefd[2])
{
    int fd0, fd1;
    struct inode *inode;
    struct dentry *dentry;
    struct file *file0, *file1;

    for (fd0 = 0; fd0 < OPEN_MAX; fd0++)
        if (current_task->fd[fd0].file == NULL)
            break;
    for (fd1 = fd0+1; fd1 < OPEN_MAX; fd1++)
        if (current_task->fd[fd1].file == NULL)
            break;
    if (fd1 == OPEN_MAX)
        return -EMFILE; /* Too many open files */

    inode = pipe_inode_create();
    if (!inode)
        return -1;

    /* Inode allocated */
    file0 = fs_file_alloc();
    file1 = fs_file_alloc();
    if (!file0 || !file1)
        return -1;

    dentry = dentry_create("", NULL, NULL);
    if (dentry == NULL)
        return -1;
    dentry->inode = inode;

    file0->flags = O_RDONLY;
    file0->refs = 1;
    file0->offset = 0;
    file0->dentry = dentry;
    *file1 = *file0;
    file1->flags = O_WRONLY;

    current_task->fd[fd0].file = file0;
    current_task->fd[fd1].file = file1;

    pipefd[0] = fd0;
    pipefd[1] = fd1;
    return 0;
}
