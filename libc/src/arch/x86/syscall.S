.intel_syntax noprefix
.section .text
.extern errno

.global syscall
syscall:
    push    ebx
    push    esi
    push    edi
    push    ebp
    mov     eax, [esp+20]   /* syscall nr */
    mov     ebx, [esp+24]   /* arg 1 */
    mov     ecx, [esp+28]   /* arg 2 */
    mov     edx, [esp+32]   /* arg 3 */
    mov     esi, [esp+36]   /* arg 4 */
    mov     edi, [esp+40]   /* arg 5 */
    mov     ebp, [esp+44]   /* arg 6 */
    int     0x80 
    pop     ebp
    pop     edi
    pop     esi
    pop     ebx
    test    eax, eax
    jns     1f
    neg     eax
    mov     dword ptr errno, eax
    mov     eax, -1
1:  ret
