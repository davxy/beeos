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
 * @file vsnprintf.c
 *
 * @brief
 *
 * @author     Davide Galassi
 * @date      Jul 15, 2014
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#define SPECIFIER_NONE           0
#define SPECIFIER_FLAGS          1
#define SPECIFIER_WIDTH          2
#define SPECIFIER_PRECISION      3
#define SPECIFIER_TYPE           4

#define FLAG_PLUS                (1 << 0)
#define FLAG_MINUS               (1 << 1)
#define FLAG_SPACE               (1 << 2)
#define FLAG_ALTERNATE           (1 << 3)
#define FLAG_ZERO                (1 << 4)



static size_t uimaxtoa(unsigned long integer, char *buffer, int base,
                       int uppercase, size_t size);

static size_t imaxtoa(long integer, char *buffer, int base,
                      int uppercase, size_t size);

static size_t simaxtoa(long integer, char *buffer, int base,
                       int uppercase, size_t size);

static size_t uimaxtoa_fill(unsigned long integer, char *buffer, int base,
                            int uppercase, int width, int filler, int max);

static size_t imaxtoa_fill(long integer, char *buffer, int base,
                           int uppercase, int width, int filler, int max);

static size_t simaxtoa_fill(long integer, char *buffer, int base,
                            int uppercase, int width, int filler, int max);

static size_t strtostr_fill(char *string, char *buffer, int width,
                            int filler, int max);

/*
 * We produce at most 'size-1' characters, + '\0'.
 * 'size' is used also as the max size for internal strings, but only if
 * it is not too big.
 */
int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int f = 0;
    int s = 0;
    int n;
    int remain = size - 1;
    int specifier = SPECIFIER_NONE;
    int flags = 0;
    int alignment;
    int filler;
    signed long     value_i;
    unsigned long     value_ui;
    char *value_cp;
    size_t width;
    size_t precision;
    size_t str_size = (size > (BUFSIZ / 2) ? (BUFSIZ / 2) : size);
    char width_str[str_size];
    char precision_str[str_size];
    int w;
    int p;

    width_str[0] = '\0';
    precision_str[0] = '\0';

    while (format[f] != 0 && s < (size-1)) {
        switch (specifier) {
        case SPECIFIER_NONE:
            if (format[f] != '%') {
                str[s++] = format[f++];
                remain--;
            } else if (format[f+1] == '%') {
                f+=2;
                str[s++] = '%';
                remain--;
            } else {
                f++;
                specifier = SPECIFIER_FLAGS;
            }
            continue;

        case SPECIFIER_FLAGS:
            switch (format[f++]) {
            case '+':
                flags |= FLAG_PLUS;
                break;
            case '-':
                flags |= FLAG_MINUS;
                break;
            case ' ':
                flags |= FLAG_SPACE;
                break;
            case '#':
                flags |= FLAG_ALTERNATE;
                break;
            case '0':
                flags |= FLAG_ZERO;
                break;
            default:
                f--;
                specifier = SPECIFIER_WIDTH;
                break;
            }
            break;

        case SPECIFIER_WIDTH:
            w = 0;
            while ('0' <= format[f] && format[f] <= '9' && w < str_size)
                width_str[w++] = format[f++];
            width_str[w] = '\0';

            if (format[f] == '.') {
                f++;
                specifier = SPECIFIER_PRECISION;
            } else {
                specifier = SPECIFIER_TYPE;
            }
            break;

        case SPECIFIER_PRECISION:
            p = 0;
            while ('0' <= format[f] && format[f] <= '9' && p < str_size)
                precision_str[p++] = format[f++];
            precision_str[p] = '\0';
            specifier = SPECIFIER_TYPE;
            break;

        case SPECIFIER_TYPE:
            n = 0;
            width = atoi(width_str);
            precision = atoi(precision_str);
            filler = (flags & FLAG_ZERO) ? '0' : ' ';
            if (flags & FLAG_SPACE) filler = ' ';
            alignment = width;
            if (flags & FLAG_MINUS) {
                alignment = -alignment;
                filler = ' '; // The filler cannot be zero, so it is black
            }

            switch (format[f++]) {
            case 'p':
                alignment = 2*sizeof(void *);
                filler = '0';
                /* no break */
            case 'x':
                value_ui = va_arg (ap, unsigned int);
                n = uimaxtoa_fill(value_ui, &str[s], 16, 0,
                                  alignment, filler, remain);
                break;
            case 'X':
                value_ui = va_arg (ap, unsigned int);
                n = uimaxtoa_fill(value_ui, &str[s], 16, 1,
                            alignment, filler, remain);
                break;
            case 'd':
            case 'i':
                /* signed int base 10. */
                value_i = va_arg (ap, int);
                if (flags & FLAG_PLUS) {
                    n = simaxtoa_fill(value_i, &str[s], 10, 0,
                                      alignment, filler, remain);
                } else {
                    n = imaxtoa_fill(value_i, &str[s], 10, 0,
                                     alignment, filler, remain);
                }
                break;
            case 'u':
                 /* unsigned int base 10. */
                value_ui = va_arg (ap, unsigned int);
                n = uimaxtoa_fill(value_ui, &str[s], 10, 0,
                                  alignment, filler, remain);
                break;
            case 'h':
                switch (format[f++]) {
                case 'h':
                    switch (format[f++]) {
                    case 'd':
                    case 'i':
                        /* signed char, base 10. */
                        value_i = va_arg (ap, int);
                        if (flags & FLAG_PLUS) {
                            n = simaxtoa_fill(value_i, &str[s],
                                10, 0, alignment, filler, remain);
                        } else {
                            n = imaxtoa_fill(value_i, &str[s],
                                10, 0, alignment, filler, remain);
                        }
                        break;
                    case 'u':
                        /* unsigned char, base 10. */
                        value_ui = va_arg (ap, unsigned int);
                        n = uimaxtoa_fill(value_ui, &str[s], 10, 0,
                                alignment, filler, remain);
                        break;
                    case 'o':
                        /* unsigned char, base 8. */
                        value_ui = va_arg (ap, unsigned int);
                        n = uimaxtoa_fill(value_ui, &str[s], 8, 0,
                                alignment, filler, remain);
                        break;
                    case 'x':
                        /* unsigned char, base 16. */
                        value_ui = va_arg (ap, unsigned int);
                        n = uimaxtoa_fill(value_ui, &str[s], 16, 0,
                                alignment, filler, remain);
                        break;
                    case 'X':
                        /* unsigned char, base 16. */
                        value_ui = va_arg (ap, unsigned int);
                        n = uimaxtoa_fill(value_ui, &str[s], 16, 1,
                                alignment, filler, remain);
                        break;
                    case 'b':
                        /* base 2 (extension) */
                        value_ui = va_arg (ap, unsigned int);
                        n = uimaxtoa_fill (value_ui, &str[s], 2, 0,
                                alignment, filler, remain);
                        break;
                    default:
                        /* unknown specifier */
                        break;
                    } /* end switch format[f+2] */
                    break;
                default:
                    break;
                } /* end switch format[f+1] */
                break;
            case 's':
                value_cp = va_arg(ap, char *);
                filler = ' ';
                n = strtostr_fill(value_cp, &str[s],
                        alignment, filler, remain);
                break;
            case 'c':
                value_ui = va_arg(ap, unsigned int);
                str[s++] = (char)value_ui;
                break;
            default:
                break;
            } /* end switch format[f] */
            specifier = SPECIFIER_NONE;
            s += n;
            remain -= n;
        break;

        default:
            break;
        } /* end switch specifier */
    }
    str[s] = '\0';
    return s;
}

/* Convert a maximum rank integer into a string. */
static size_t uimaxtoa(unsigned long integer, char *buffer, int base,
                       int uppercase, size_t size)
{
    unsigned long  integer_copy = integer;
    size_t  digits;
    int  b;
    unsigned char  remainder;

    for (digits = 0; integer_copy > 0; digits++)
        integer_copy = integer_copy / base;

    if (buffer == NULL && integer == 0)
        return 1;
    if (buffer == NULL && integer > 0)
        return digits;

    if (integer == 0) {
        buffer[0] = '0';
        //buffer[1] = '\0';
        return 1;
    }

    /* Fix the maximum number of digits. */
    if (size > 0 && digits > size)
        digits = size;

    //buffer[digits] = '\0';            // End of string.

    for (b = digits - 1; integer != 0 && b >= 0; b--) {
        remainder = integer % base;
        integer   = integer / base;

        if (remainder <= 9) {
            *(buffer + b) = remainder + '0';
        } else {
            if (uppercase)
                *(buffer + b) = remainder - 10 + 'A';
            else
                *(buffer + b) = remainder - 10 + 'a';
        }
    }

    return digits;
}



/* Convert a maximum rank integer with sign into a string. */
static size_t imaxtoa(long integer, char *buffer, int base,
                      int uppercase, size_t size)
{
    if (integer >= 0)
        return uimaxtoa(integer, buffer, base, uppercase, size);

    /* At this point, there is a negative number, less than zero. */
    if (buffer == NULL)
        return uimaxtoa(-integer, NULL, base, uppercase, size) + 1;

    *buffer = '-';        // The minus sign is needed at the beginning.
    if (size == 1) {
        //*(buffer + 1) = '\0';
        return 1;
    } else {
        return uimaxtoa(-integer, buffer+1, base, uppercase, size-1) + 1;
    }
}

/*
 * Convert a maximum rank integer with sign into a string, placing the sign
 * also if it is positive.
 */
static size_t simaxtoa(long integer, char *buffer, int base,
                       int uppercase, size_t size)
{
    if (buffer == NULL && integer >= 0)
        return uimaxtoa(integer, NULL, base, uppercase, size) + 1;

    if (buffer == NULL && integer < 0)
        return uimaxtoa(-integer, NULL, base, uppercase, size) + 1;

    // At this point, `buffer' is different from NULL.
    if (integer >= 0)
        *buffer = '+';
    else
        *buffer = '-';

    if (size == 1) {
        //*(buffer + 1) = '\0';
        return 1;
    }

    if (integer >= 0)
        return uimaxtoa(integer, buffer+1, base, uppercase, size-1) + 1;
    else
        return uimaxtoa(-integer, buffer+1, base, uppercase, size-1) + 1;
}

/*
 * Convert a maximum rank integer without sign into a string, taking care of
 * the alignment.
 */
static size_t uimaxtoa_fill(unsigned long integer, char *buffer, int base,
                            int uppercase, int width, int filler, int max)
{
    size_t size_i;
    size_t size_f;

    if (max < 0)
        return 0;   // "max" must be a positive value

    size_i = uimaxtoa(integer, NULL, base, uppercase, 0);

    if (width > 0 && max > 0 && width > max)
        width = max;
    if (width < 0 && -max < 0 && width < -max)
        width = -max;

    if (size_i > abs(width))
        return uimaxtoa(integer, buffer, base, uppercase, abs(width));

    if (width == 0 && max > 0)
        return uimaxtoa(integer, buffer, base, uppercase, max);

    if (width == 0)
        return uimaxtoa(integer, buffer, base, uppercase, abs(width));

    // size_i <= abs(width).

    size_f = abs(width) - size_i;

    if (width < 0) {
        /* Left alignment. */
        uimaxtoa(integer, buffer, base, uppercase, 0);
        memset(buffer + size_i, filler, size_f);
    } else {
        /* Right alignment. */
        memset(buffer, filler, size_f);
        uimaxtoa(integer, buffer + size_f, base, uppercase, 0);
    }

    //buffer[abs(width)] = '\0';

    return abs(width);
}

/*
 * Convert a maximum rank integer with sign into a string, takeing care of the
 * alignment.
 */
static size_t imaxtoa_fill(long integer, char *buffer, int base,
                           int uppercase, int width, int filler, int max)
{
    size_t size_i;
    size_t size_f;

    //int abs_width;

    if (max < 0)
        return 0; /* 'max' must be a positive value. */

    size_i = imaxtoa(integer, NULL, base, uppercase, 0);

    if (width > 0 &&  max > 0 && width >  max)
        width =  max;
    if (width < 0 && -max < 0 && width < -max)
        width = -max;

    if (size_i > abs(width))
        return imaxtoa(integer, buffer, base, uppercase, abs (width));

    if (width == 0 && max > 0)
        return imaxtoa(integer, buffer, base, uppercase, max);

    if (width == 0)
        return imaxtoa(integer, buffer, base, uppercase, abs (width));

    // size_i <= abs(width).

    size_f = abs(width) - size_i;

    if (width < 0) {
        /* Left alignment. */
        imaxtoa(integer, buffer, base, uppercase, 0);
        memset(buffer + size_i, filler, size_f);
    } else {
        /* Right alignment. */
        memset(buffer, filler, size_f);
        imaxtoa(integer, buffer + size_f, base, uppercase, 0);
    }
    //buffer[abs(width)] = '\0';

    return abs(width);
}

/*
 * Convert a maximum rank integer with sign into a string, placing the sign
 * also if it is positive and taking care of the alignment.
 */
static size_t simaxtoa_fill(long integer, char *buffer, int base,
                            int uppercase, int width, int filler, int max)
{
    size_t size_i;
    size_t size_f;

    if (max < 0)
        return 0; /* 'max' must be a positive value. */

    size_i = simaxtoa(integer, NULL, base, uppercase, 0);

    if (width > 0 &&  max > 0 && width >  max)
        width =  max;
    if (width < 0 && -max < 0 && width < -max)
        width = -max;

    if (size_i > abs (width))
        return simaxtoa(integer, buffer, base, uppercase, abs (width));

    if (width == 0 && max > 0)
        return simaxtoa(integer, buffer, base, uppercase, max);

    if (width == 0)
        return simaxtoa(integer, buffer, base, uppercase, abs (width));

    // size_i <= abs(width).
    size_f = abs(width) - size_i;

    if (width < 0) {
        /* Left alignment. */
        simaxtoa(integer, buffer, base, uppercase, 0);
        memset(buffer + size_i, filler, size_f);
    } else {
        /* Right alignment. */
        memset(buffer, filler, size_f);
        simaxtoa(integer, buffer + size_f, base, uppercase, 0);
    }
    //buffer[abs(width)] = '\0';

    return abs(width);
}

/** Transfer a string with care for the alignment. */
static size_t strtostr_fill(char *string, char *buffer, int width,
                            int filler, int max)
{
    size_t size_s;
    size_t size_f;

    if (max < 0)
        return 0;   /* 'max' must be a positive value. */

    size_s = strlen(string);

    if (width > 0 &&  max > 0 && width >  max)
        width =  max;
    if (width < 0 && -max < 0 && width < -max)
        width = -max;

    if (width != 0 && size_s > abs(width)) {
        memcpy(buffer, string, abs(width));
        buffer[width] = '\0';
        return width;
    }

    if (width == 0 && max > 0 && size_s > max) {
        memcpy(buffer, string, max);
        buffer[max] = '\0';
        return max;
    }

    if (width == 0 && max > 0 && size_s < max) {
        memcpy(buffer, string, size_s);
        buffer[size_s] = '\0';
        return size_s;
    }

    /* width != 0 */
    // size_s <= abs (width)

    size_f = abs(width) - size_s;

    if (width >= 0) {
        /* Right alignment. */
        memset(buffer, filler, size_f);
        strncpy(buffer+size_f, string, size_s);
    } else {
        /* Left alignment. */
        strncpy(buffer, string, size_s);
        memset(buffer+size_s, filler, size_f);
    }
    buffer[abs(width)] = '\0';

    return abs(width);
}
