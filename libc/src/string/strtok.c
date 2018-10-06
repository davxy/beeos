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
 * @file    strcpy.c
 *
 * @brief
 *
 * @author    dave
 * @date    Jul 20, 2014
 */

#include <string.h>

char *strtok(char *str, const char *delim)
{
    static char *next = NULL;
    size_t i = 0;
    size_t j;
    int found_token;
    int found_delim;

    /*
     * If the string received a the first parameter is a null pointer,
     * the static pointer is used. But if it is already NULL, the scan
     * cannot start.
     */
    if (str == NULL) {
        if (next == NULL)
            return NULL;
        else
            str = next;
    }

    /*
     * If the string received as the first parameter is empty, the scan
     * cannot start.
     */
    if (str[0] == 0) {
        next = NULL;
        return NULL;
    } else {
        if (delim[0] == 0)
            return str;
    }

    /* Find the next token. */
    for (i = 0, found_token = 0, j = 0;
         str[i] != 0 && !found_token; i++) {
        /* Look inside delimiters. */
        for (j = 0, found_delim = 0; delim[j] != 0; j++) {
            if (str[i] == delim[j])
                found_delim = 1;
        }
        /*
         * If current character inside the string is not a delimiter,
         * it is the start of a new token.
         */
        if (!found_delim) {
            found_token = 1;
            break;
        }
    }
    /*
     * If a token was found, the pointer is updated.
     * If otherwise the token is not found, this means that there
     * are no more.
     */
    if (found_token) {
        str += i;
    } else {
        next = NULL;
        return NULL;
    }

    /* Find the end of the token. */
    for (i = 0, found_delim = 0; str[i] != 0; i++) {
        for (j = 0; delim[j] != 0; j++) {
            if (str[i] == delim[j]) {
                found_delim = 1;
                break;
            }
        }
        if (found_delim)
            break;
    }

    /*
     * If a delimiter was found, the corresponding character must be
     * reset to zero. If otherwise the string is terminated, the
     * scan is terminated.
     */
    if (found_delim) {
        str[i] = 0;
        next = &str[i+1];
    } else {
        next = NULL;
    }

    /* At this point, the current string represent the token found. */
    return str;
}
