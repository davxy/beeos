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

/* libc */
#include <stdio.h>
#include <sys/wait.h>
/* libu */
#include <err.h>
#include <const.h>

char line[MAXLINE];

int main(void)
{
    FILE *fpin;

    if ((fpin = popen("/test/hello", "r")) == NULL)
        err_sys("popen error");
    while (1) {
        fputs("prompt> ", stdout);
        fflush(stdout);
        if (fgets(line, MAXLINE, fpin) == NULL)
            break;
        if (fputs(line, stdout) == EOF)
            err_sys("fputs error to pipe");
        putchar('\n');
    }
    if (pclose(fpin) == -1)
        err_sys("pclose error");
    putchar('\n');
    return 0;
}
