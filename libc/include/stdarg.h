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

#ifndef _STDARG_H_
#define _STDARG_H_

typedef unsigned char *va_list;

#define va_start(ap, last) \
    ((ap) = ((va_list) &(last)) + (sizeof (last)))

#define va_end(ap) \
    ((ap) = (va_list)0)

#define va_copy(dest, src) \
    ((dest) = (va_list) (src))

#define va_arg(ap, type) \
     (((ap) = (ap) + (sizeof (type))), *((type *) ((ap) - (sizeof (type)))))

#endif /* _STDARG_H_ */
