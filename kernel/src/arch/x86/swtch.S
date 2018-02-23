/*
 * swtch.S
 *
 *  Created on: Feb 23, 2018
 *      Author: davxy
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
