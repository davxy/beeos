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

/**
 * @brief   System V message queues.
 *
 * A message queue is a linked list of messages stored within the kernel and
 * identified by a message queue identifier.
 */

#ifndef BEEOS_IPC_MSG_H_
#define BEEOS_IPC_MSG_H_

#include "sync/cond.h"
#include <sys/types.h>

typedef unsigned short msgqnum_t;
typedef unsigned short msglen_t;

struct ipc_perm
{
    uid_t uid;      /**< Owner's effective used ID */
    gid_t gid;      /**< Owner's effective group ID */
    uid_t cuid;     /**< Creator's effective used ID */
    gid_t cgid;     /**< Craator's effective group ID */
    mode_t mode;    /**< Access mode */
};

struct msg
{
    struct msg *msg_next;   /**< Next message on queue */
    long        msg_type;
    char        *msg_spot;  /**< Message text address */
    msglen_t    msg_ts;     /**< Message text size */
};

struct msqid_ds
{
    struct ipc_perm msg_perm;   /**< Permissions and owner */
    msgqnum_t       msg_qnum;   /**< Number of messages on queue */
    msglen_t        msg_qbytes; /**< Max number of bytes on queue */
    pid_t           msg_lspid;  /**< Pid of last msgsnd() */
    pid_t           msg_lrpid;  /**< Pid of last msgrcv() */
    time_t          msg_stime;  /**< Last msgsnd() time */
    time_t          msg_rtime;  /**< Last msgrcv() time */
    time_t          msg_ctime;  /**< Last change time */
    struct msg      *msg_first; /**< First message on queue */
    struct msg      *msg_last;  /**< Last message on queue */
    struct cond     *wwait;
    struct cond     *rwait;
    struct htable_link hlink;
};

#endif /* BEEOS_IPC_MSG_H_ */

