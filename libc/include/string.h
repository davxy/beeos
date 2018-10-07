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
 * @file    string.h
 * @brief
 * @author    Davide Galassi
 * @date    Jul 12, 2014
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <sys/types.h>
#include <stddef.h>


/**
 * Compares the first n bytes (each interpreted as unsigned char) of the memory
 * areas s1 and s2.
 *
 * Returns an integer less than, equal to, or greater than zero if the first
 * n bytes of s1 is found, respectively, to be less than, to match, or be
 * greater than the first n bytes of s2.
 * For a nonzero return value, the sign is determined by the sign of the
 * difference between the first pair of bytes (interpreted as unsigned char)
 * that differ in s1 and s2.
 * If n is zero, the return value is zero.
 *
 * @param s1    First memory region.
 * @param s2    Second memory region pointer.
 * @param n     Number of bytes to compare.
 * @return      0 if equal, <0 if s1 < s2 and >0 is s1 > s2.
 */
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * Copies n bytes from memory area src to memory area dst. The memory areas
 * must not overlap. Use memmove if the memory areas do overlap.
 *
 * @param dst
 *     Destination memory pointer.
 * @param src
 *     Source memory pointer.
 * @param n
 *     Number of bytes to copy.
 * @return
 *  Pointer to dst.
 */
void *memcpy(void *dst, const void *src, size_t n);

/**
 * Copies n bytes from memory area src to memory area dst.  The memory areas
 * may overlap.
 * @param dst
 *  Destination memory pointer.
 * @param src
 *  Source memory pointer.
 * @param n
 *  Number of bytes to move.
 * @return
 *     A pointer to dst.
 */
void *memmove(void *dst, const void *src, size_t n);

/**
 * Calculates the length of the string s, excluding the terminating null byte.
 * @param s
 *     Input string.
 * @return
 *  Number of bytes in the string s.
 */
size_t strlen(const char *s);

/**
 * Fills memory with a constant byte
 * @param s
 *     Pointero to the destination memory area.
 * @param c
 *     Constant byte.
 * @param n
 *     Number of bytes to set.
 * @return
 *     A pointer to the memory area s.
 */
void *memset(void *s, int c, size_t n);

/**
 * Compares two input strings.
 *
 * @param s1
 *  First null terminated string.
 * @param s1
 *  Second null terminated string.
 */
int strcmp(const char *s1, const char *s2);

/**
 * Compares (at most) n bytes of two input strings.
 *
 * @param s1
 *  First null terminated string.
 * @param s1
 *  Second null terminated string.
 * @param n
 *  Max number of bytes to compare.
 */
int strncmp(const char *s1, const char *s2, size_t n);

/**
 *
 * @param dst
 * Destination string.
 * @param src
 *     Source string.
 * @return
 *     A pointer to the destination string dst.
 */
char *strcpy(char *dst, const char *src);

/**
 * Similar to the strcpy() function except that at most n bytes of src are
 * copied. Warning: If there is no null byte among the first n bytes of src,
 * the string placed in dst will not be null-terminated.
 * @param dst
 *     Destination string.
 * @param src
 *     Source string.
 * @param n
 *     Number of bytes to copy.
 * @return
 *     A pointer to the destination string dst.
 */
char *strncpy(char *dst, const char *src, size_t n);

char *strcat(char *dst, const char *src);


/**
 * Returns a pointer to a string that describes the error code passed in the
 * argument errnum. This string must not be modified by the application, but
 * may be modified by a subsequent call to strerror. No other library function,
 * including perror, will modify this stirng.
 *
 * @param errnum
 *  Error code, usually stored by errno.
 * @return
 *  An appropriate error description string, or an "Unknwon error nnn" message
 *  if the error number is unknown.
 */
char *strerror(int errnum);


char *strtok(char *str, const char *delim);

char *strchr(const char *str, int c);

char *strdup(const char *s);


#endif /* _STRING_H_ */
