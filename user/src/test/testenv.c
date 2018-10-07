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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void printenv(const char *msg)
{
    int i;

    printf("------------------------------\n");
    printf("%s\n", msg);
    printf("environ @ %p\n", environ);
    for (i = 0; environ[i]; i++)
        printf(" [%d] %s @ %p\n", i, environ[i], environ[i]);
}


int main(void)
{
    printenv("Start status");

    if (unsetenv("USER") < 0) {
        perror("unsetenv USER");
        return 1;
    }
    printenv("After 'unsetenv' of USER (existing)");

    if (setenv("FOO", "bar", 1) < 0) {
        perror("setenv");
        return 1;
    }
    printenv("After 'setenv' FOO (unexisting)");

    if (setenv("HOME", "/home/zz", 1) < 0) {
        perror("setenv HOME");
        return 1;
    }
    printenv("After 'setenv' HOME (existing, shorter)");

    if (setenv("HOME", "/home/Hikr", 1) < 0) {
        perror("setenv HOME");
        return 1;
    }
    printenv("After 'setenv' HOME (existing, longer)");

    return 0;
}
