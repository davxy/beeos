#ifndef _ARCH_ARM_ISR_H_
#define _ARCH_ARM_ISR_H_

struct isr_frame
{
    int     int_no;
};

#define ISR_SYSCALL 128

#endif /* _ARCH_ARM_ISR_H_ */
