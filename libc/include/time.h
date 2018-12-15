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

#ifndef _TIME_H_
#define _TIME_H_

typedef long long int time_t;
typedef long long int clock_t;

struct timespec {
    time_t  tv_sec;     /**> Seconds */
    long    tv_nsec;    /**> Nanoseconds */
};

#define CLOCKS_PER_SEC ((clock_t) 100)

clock_t clock(void);

#endif /* _TIME_H_ */
