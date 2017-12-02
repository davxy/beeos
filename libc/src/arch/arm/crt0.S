.section .text

.extern main
.extern exit
.globl _start

_start:
    bl      main
    bl      exit
