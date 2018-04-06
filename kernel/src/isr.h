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


#ifndef BEEOS_ISR_H_
#define BEEOS_ISR_H_

#include "arch/x86/isr_arch.h"


void isr_handler(struct isr_frame *ifr);

typedef void (*isr_handler_t)(void);

void isr_register_handler(unsigned int num, isr_handler_t func);

void isr_init(void);

#endif /* BEEOS_ISR_H_ */
