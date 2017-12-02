.intel_syntax noprefix
.section .text

.extern main
.extern exit
.extern environ

.global _start
.type _start, @function
_start:
    /* Save arguments */
    pop     eax     /* argc */
    pop     ebx     /* argv */
    pop     ecx     /* envp */
    /* Set up end of stack frame */
    push    0       /* return address */
    push    0       /* frame pointer */
    mov     ebp, esp   

    mov     environ, ecx

    push    ecx     /* envp */
    push    ebx     /* argv */
    push    eax     /* argc */

#    call    libc_init   /* initialize standard library */
    
    call    main
        
    /* Terminate the process with the exit code */
    push    eax     /* return value */
    call    exit
.size _start, .-_start
