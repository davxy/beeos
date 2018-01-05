#ifndef _ARCH_ARM_TASK_H_
#define _ARCH_ARM_TASK_H_

#include "isr.h"

struct task_arch
{
    struct isr_frame *ifr;
    struct isr_frame *sfr;
};

#endif /* _ARCH_ARM_TASK_H_ */
