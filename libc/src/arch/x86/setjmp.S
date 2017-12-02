.section .text
.intel_syntax noprefix

.global setjmp
setjmp:
    mov edx, [esp+4]    /* Get the pointer to the jmp_buf */
    mov [edx+0x00], ebx
    mov [edx+0x04], esi
    mov [edx+0x08], edi
    mov [edx+0x0C], esp
    mov [edx+0x10], ebp
    mov eax, [esp]      /* get the return address from the stack */
    mov [edx+0x14], eax
    xor eax, eax
    ret

.global longjmp
longjmp:
    mov edx, [esp+4]    /* Pointer to the jmp_buf */
    mov ebx, [edx+0x00]
    mov esi, [edx+0x04]
    mov edi, [edx+0x08]
    mov esp, [edx+0x0C]
    mov ebp, [edx+0x10]
    mov eax, [edx+0x14] /* Saved instruction pointer */
    mov [esp], eax      /* Change the function return value */
    mov eax, [esp+8]    /* Get the return value */
    ret
