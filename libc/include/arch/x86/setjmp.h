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

#ifndef _ARCH_X86_SETJMP_H_
#define _ARCH_X86_SETJMP_H_

#include <stdint.h>

/**
 * Registers to be saved, by the called, across function calls according
 * to the cdecl calling convention. The registers EAX,ECX and EDX are
 * caller-saved.
 */
struct jmp_buf_arch {
    uint32_t    ebx;
    uint32_t    esi;
    uint32_t    edi;
    uint32_t    esp;
    uint32_t    ebp;
    uint32_t    eip;
};

#define JMP_BUF_SIZE    sizeof(struct jmp_buf_arch)

#endif /* _ARCH_X86_SETJMP_H_ */
