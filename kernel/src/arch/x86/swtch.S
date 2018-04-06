/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
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

.intel_syntax noprefix

.section .text

/*
 * void swtch(struct context **old, struct context *new);
 */
.globl swtch
swtch:
    mov     eax, [esp + 4]
    mov     edx, [esp + 8]
    /* Save old callee-save registers */
    /* Instruction pointer saved implicitly by function "call" */
    push    ebp
    push    ebx
    push    esi
    push    edi
    /* Switch stacks */
    mov     [eax], esp
    mov     esp, edx
    /* Restore new context */
    pop     edi
    pop     esi
    pop     ebx
    pop     ebp
    ret

