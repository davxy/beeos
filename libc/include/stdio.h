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
 * @file    stdio.h
 * @brief
 * @author     Davide Galassi
 * @date      Jul 15, 2014
 */

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>
#include <sys/types.h>
#include <stddef.h>


/** IO buffer size. */
#define BUFSIZ 512

/** The End Of File character */
#define EOF (-1)

/** Buffering mode */
#define _IONBF  0
#define _IOLBF  1
#define _IOFBF  2


/** Opaque FILE type */
typedef struct _FILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/**
 * Produce output for the stdout according to the format string.
 * @param format
 *     Format string.
 * @return
 *     Number of characters printed (excluding the null byte used to end output
 *     to strings).
 */
int printf(const char *format, ...);

/**
 *
 */
int sprintf(char *str, const char *fmt, ...);

/**
 *
 */
int snprintf(char *str, size_t size, const char *fmt, ...);

/**
 * Produce output to be stored in an output string according to the format
 * string.
 * @param str
 *
 */
int vsprintf(char *str, const char *format, va_list ap);

/**
 *
 * @param format
 * @param ap
 * @return
 */
int vprintf(const char *format, va_list ap);

/**
 *
 * @param str
 * @param n
 * @param format
 * @param ap
 * @return
 */
int vsnprintf(char *str, size_t n, const char *format, va_list ap);

int fprintf(FILE *stream, const char *format, ...);

int vfprintf(FILE *stream, const char *format, va_list ap);


int fputc(int c, FILE *stream);

#define putc fputc

int putchar(int c);

int fputs(const char *str, FILE *stream);

int puts(const char *str);

int fgetc(FILE *stream);

#define getc fgetc

int getchar(void);

/**
 * Reads ar most one less than 'size' characters from 'stream' and stores them
 * into the buffer pointer by 'str'. Reading stops after an EOF or a newline.
 * If a newline is read, it is stored into the buffer. A terminating null
 * byte ('\0') is stored after the last character in the buffer.
 *
 * @param str   Ouput buffer pointer.
 * @param size  Size of output buffer.
 * @param stream    Input stream.
 * @return      Returns 'str' on success, and NULL on error or when end of
 *              file occurs while no characters have been read.
 */
char *fgets(char *str, int size, FILE *stream);

/**
 * Reads a string from 'stdin' into the buffer pointer by 'str' until either
 * a terminating newline of EOF, which it replaces with a null byte ('\0').
 * No check for buffer overrun is performed.
 *
 * @param str   Output buffer pointer.
 * @return      Retuns 'str' on success, and NULL on error or when end of
 *              file occurs while no characters have been read.
 */
char *gets(char *str);

/**
 * Produces a message on standard error describing the last error encountered
 * during a call to a system or library function.
 *
 * @param str   If str is not NULL or *str is not a null byte, the argument
 *              str is printed followed by a colon and a blank before the
 *              error description.
 */
void perror(const char *str);

/**
 * Opens the file whose name is the string pointed to by path and associates
 * a stream with it.
 * The argument 'mode' points to a string beginning with one of the following:
 * - r      Open file for reading. Stream positioned at the beginning.
 * - r+     Open for reading and writing. Stram positioned at the beginning.
 * - w      Open for writing. Truncate to zero length or create. Stream
 *          positioned at the beginning.
 * - w+     Open for reading and writing. Truncate to zero length or create.
 *          Stream positioned at the beginning.
 * - a      Open for appending. The file is created if does not exist. Stream
 *          positioned at the end of file.
 * - a+     Open for reading and appending. The file is created if does not
 *          exist. Stream positioned at the end of file.
 * Any created files will have mode 0666.
 *
 * @param path  File name path. Relative or absolute.
 * @param mode  Open mode string. See the long description for details.
 * @return      On success returns a FILE pointer. Otherwise, NULL and 'errno'
 *              is set to indicate the error.
 */
FILE *fopen(const char *path, const char *mode);

/**
 * Associates a stream with an existing file descriptor. The mode of the
 * stream must be compatible with the mode of the file descriptor.
 * The error and EOF indicators are cleared.
 * The file is not dup'ed, and will be closed when the created stream will
 * be closed.
 *
 * @param fd    File descriptor.
 * @mode        Open mode sctring. See fopen() for details.
 * @return      On success returns a FILE pointer. Otherwise, NULL and 'errno'
 *              is set to indicate the error.
 */
FILE *fdopen(int fd, const char *mode);

/**
 * Flushes the stream pointed to by 'stream' (using fflush()) and closes the
 * underlying file descriptor.
 *
 * @param stream    Stream pointer.
 * @return          On success 0 is returned. Otherwise, EOF is returned
 *                  and errno is set to indicate the error.
 */
int fclose(FILE *stream);

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

int ferror(FILE *stream);

int feof(FILE *stream);

/**
 * For output streams, fflush() forces a write of all user-space buffered
 * data for the given ouput.
 * For input streams associated with seekable files, fflush discards any
 * buffered data that has been fetched from the underlying file, but has
 * not been consumed by the application.
 *
 * @param stream    Stream pointer.
 * @return          On success 0 is returned. Otherwise, EOF is returned
 *                  abd errno is set to indicate the error.
 */
int fflush(FILE *stream);

/**
 * Opens a process by creating a pipe, forking, and invoking the shell.
 * Since the pipe is by definition unidirectional, the 'type' argument
 * may specify only reading or writing, not both.
 * The 'command' argument is passed to '/bin/sh' using -c flag. This means
 * that the shell expands any of its special characters in command.
 *
 * @param command   Command to be executed
 * @param type      Either the letter 'r' for reading or 'w' for writing.
 * @return          A pointer to an open stream. Must be closed by pclose.
 */
FILE *popen(const char *command, const char *type);

/**
 * Waits for the associated process to terminate and returns the exit
 * status of the commans as returned by wait.
 *
 * @param stream    Stream to be closed.
 * @return          On success the exit status of the command, -1 on error.
 */
int pclose(FILE *stream);

/**
 * Returns its integer file descriptor.
 *
 * @param stream    Stream pointer.
 * @return          Integer file descriptor.
 */
int fileno(FILE *stream);


#endif /* _STDIO_H_ */
