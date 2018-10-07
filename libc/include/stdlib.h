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

/**
 * @file    stdlib.h
 * @brief
 * @author     Davide Galassi
 * @date      Jul 18, 2014
 */

#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <sys/types.h>

#define ATEXIT_MAX  32

/**
 * Converts the initial portion of the string pointed to by str to int.
 *
 * @param str   Pointer to the string to convert.
 * @return      The converted value.
 */
int atoi(const char *str);

/**
 * Behaves the same as atoi(), except that they convert the initial portion of
 * the string to their return type of long.
 *
 * @param str   Pointer to the string to convert.
 * @return      The converted value.
 */
long atol(const char *str);

/**
 * Computes the absolute value of its integer operand, i.
 * If the result cannot be represented, the behavior is undefined.
 * @param i     Integer value value.
 * @return      The absolute value of i.
 */
int abs(int i);

/**
 * Causes normal process termination and the value of the status is
 * returned to the parent (see wait(2)).
 * All functions registered with atexit(3) are called, in the reverse
 * order of their registration.
 * All open stdio(3) stream are flushed and closed.
 *
 * @param status    Exit status value.
 * @return          The function does not return.
 */
void exit(int status);

/**
 * Registers the given function to be called at normal process termination,
 * either via exit(3) or via return from the program's main().  Functions
 * so registered are called in the reverse order of their registration;
 * no arguments are passed.
 * The same function may be registered multiple times: it is  called  once
 * for each registration.
 * When a child process is created via fork(2), it inherits copies of its
 * parent registrations. Upon a successful call to one of the exec functions
 * all registrations are removed.
 *
 * @param func      Pointer the function to register.
 * @return          Zero on success.
 */
int atexit(void (* func)(void));

/**
 * Terminates the calling process "immediately".
 * Any open file descriptors belonging to the process are closed.
 * Any children of the process are inherited by init().
 * The process's parent is asent a SIGCHLD signal.
 *
 * @param status    Exit status value.
 */
void _Exit(int status);

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);


char *getenv(const char *name);

int setenv(const char *name, const char *value, int rewrite);

int unsetenv(const char *name);

int putenv(char *str);

/**
 * The system() function passes the string pointed by 'cmd' to the host
 * environment to be executed by the command processor.
 * The function uses fork() to create a child process that executes
 * the shell command specified in 'cmd' using execl() as follows:
 *      execl("/bin/sh", "sh", "-c", command, (char *)0);
 * The function returns after the command has been completed.
 * During the execution of the command, SIGCHLD will be blocked, and
 * SIGINT nad SIGQUIT will be ignored, in the process that calls system().
 */
int system(const char *cmd);

void abort(void);

#endif /* _STDLIB_H_ */
