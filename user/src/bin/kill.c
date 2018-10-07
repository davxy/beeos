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

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void usage()
{
    printf("kill: usage [-s sigspec] pid\n");
    exit(1);
}

const struct
{
    int signo;
    char *signame;
} sigmap[] = {
    { SIGHUP,    "HUP"  },
    { SIGINT,    "INT"  },
    { SIGQUIT,   "QUIT" },
    { SIGILL,    "ILL"  },
    { SIGTRAP,   "TRAP" },
    { SIGABRT,   "ABRT" },
    { SIGIOT,    "IOT"  },
    { SIGBUS,    "BUS"  },
    { SIGFPE,    "FPE"  },
    { SIGKILL,   "KILL" },
    { SIGUSR1,   "USR1" },
    { SIGSEGV,   "SEGV" },
    { SIGUSR2,   "USR2" },
    { SIGPIPE,   "PIPE" },
    { SIGALRM,   "ALRM" },
    { SIGTERM,   "TERM" },
    { SIGSTKFLT, "STKFLT" },
    { SIGCHLD,   "CHLD" },
    { SIGCONT,   "CONT" },
    { SIGSTOP,   "STOP" },
    { SIGTSTP,   "TSTP" },
    { SIGTTIN,   "TTIN" },
    { SIGTTOU,   "TTOU" },
    { SIGURG ,   "URG"  },
    { SIGXCPU,   "XCPU" },
    { SIGXFSZ,   "XFSZ" },
    { SIGVTALRM, "VTALRM" },
    { SIGPROF,   "PROF" },
    { SIGWINCH,  "WINCH" },
    { SIGIO  ,   "IO"   },
    { SIGPOLL,   "POLL" },
    { SIGPWR ,   "PWR"  },
    { SIGSYS ,   "SYS"  }
};

#define MAPSIZE (sizeof(sigmap)/sizeof(*sigmap))

int main(int argc, char *argv[])
{
    int pid, signo, i;

    if (argc < 2)
        usage();

    /* default */
    signo = SIGTERM;

    i = 1;
    if (strcmp(argv[i], "-s") == 0) {
        if (argc == 2)
            usage();
        for (i = 0; i < MAPSIZE; i++) {
            if (strcmp(sigmap[i].signame, argv[2]) == 0) {
                signo = sigmap[i].signo;
                break;
            }
        }
        i = 3;
    }
    pid = atol(argv[i]);
    if (kill(pid, signo) < 0)
        perror("kill");
    return 0;
}
