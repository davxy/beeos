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
 * @file    stddef.h
 * @brief
 * @author  Davide Galassi
 * @date    Jul 12, 2014
 */

#ifndef _STDDEF_H_
#define _STDDEF_H_


#include <sys/types.h>
#include <stdint.h>


/** Null pointer constant. */
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

/**
 * Integer constant expression of type size_t, the value of which is the
 * offset in bytes to the structure member (member-designator), from the
 * beginning of its structure (type).
 */
#define offsetof(type, memb) ((size_t)((char *)&((type *)0)->memb - (char *)0))

typedef long ptrdiff_t;

#endif /* _STDDEF_H_ */
