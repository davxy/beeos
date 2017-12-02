.intel_syntax noprefix
.section .text
.extern errno

/* TODO */

.global syscall
syscall:
    mov     rax, rdi        /* syscall nr */
    syscall
    ret
