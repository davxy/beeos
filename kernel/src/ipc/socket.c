#include "sync/spinlock.h"
#include "fs/vfs.h"
#include "proc.h"
#include "sync/cond.h"
#include "kmalloc.h"
#include "sys.h"
#include "kprintf.h"
#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

struct file *fs_file_alloc(void);

struct socket_inode
{
    struct inode base;
    struct cond  queue;
    int nrp;                /**< Next read position */
    int nwp;                /**< Next write position */
    int queued_writers;     /**< Number of stopped writers */
    int queued_readers;     /**< Number of queued readers */
    int family;
    int type;
    int protocol;
};


static int socket_read(struct inode *inod, void *buf,
                       size_t count, size_t offset)
{
    kprintf("SOCK read\n");
#if 0
    int n, left;
    char *ptr = (char *)buf;
    struct socket_inode *snode = (struct socket_inode *)inode;

    left = count;
    spinlock_lock(&snode->queue.lock);
    while (left > 0)
    {
        while (pnode->nrp == pnode->nwp) /* empty buffer */
        {
            /* no more writers: 
             * WARNING: this does not contemplate if there are a lot of readers and 0 writers.
             * TODO a modification is needed in case of dup? */
            if (pnode->base.ref == 1)
                return 0;

            /* If something has been read and there are no queued writers */
            if (left != count && pnode->queued_writers == 0) /* something has been read */
                goto done;

            //if (BLOCKING)
            pnode->queued_readers++;
            if (pnode->queued_writers > 0) /* there are pending writers */
                cond_broadcast(&pnode->queue); /* wakeup all before (eventually) sleep */
            cond_wait(&pnode->queue);
            pnode->queued_readers--;
            
            //else // unlock fisrt!!!
            //    goto spinlock_unlock;
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
    if (n > 0) /* signal if something has been read */
        cond_broadcast(&pnode->queue);
    return n;
#endif
    return 0;
}

/* TODO: this is a 'file' operation. 
 * Thus the function should take a file and as a consequence 
 * we can check if that is not blocking */
static int socket_write(struct inode *inode, const void *buf,
                        size_t count, size_t offset)
{
    kprintf("SOCK write\n");

#if 0
    /// TEMPORARY TEST
    extern struct e1000 eth;
    size_t e1000_tx(struct e1000 *, const char *, size_t);
    return e1000_tx(&eth, buf, count);

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
            if (pnode->base.ref == 1)
            {
                sys_kill(sys_getpid(), SIGPIPE);
                /* in case the signal is catched return an error */
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
    if (n > 0) /* Signal if something has been written */
        cond_broadcast(&pnode->queue);
    return n;
#endif
    return 0;
}


static const struct inode_ops socket_ops = {
    .read  = socket_read,
    .write = socket_write
};

struct socket_inode *socket_inode_create(int family,
        int type, int protocol)
{
    struct socket_inode *snode;
    snode = kmalloc(sizeof(struct socket_inode), 0);
    if (!snode)
        return NULL;
    memset(snode, 0, sizeof(*snode));
    snode->base.mode = S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO;
    snode->base.ops = &socket_ops;
    snode->base.ref = 1;
    cond_init(&snode->queue);

    snode->family = family;
    snode->type = type;
    snode->protocol = protocol;
    return snode;
}


int socket_create(int family, int type, int protocol)
{
    int fd;
    struct inode *inod;
    struct file *file;
    struct dentry *dent;

    for (fd = 0; fd < OPEN_MAX; fd++) {
        if (current->fds[fd].fil == NULL)
            break;
    }
    if (fd == OPEN_MAX)
        return -EMFILE; /* Too many open files */

    inod = (struct inode *)socket_inode_create(family, type, protocol);
    if (inod == NULL)
        return -1;

    /* Inode allocated */
    file = fs_file_alloc();
    if (file == NULL)
        return -1;

    dent = dentry_create("", NULL, NULL);
    if (dent == NULL)
        return -1;
    dent->inod = idup(inod);

    file->flags = O_RDWR;
    file->ref = 1;
    file->off = 0;
    file->dent = dent;
    dent->ref = 1;
    current->fds[fd].fil = file;

    return fd;
}
