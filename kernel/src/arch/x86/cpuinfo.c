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

#include "../../cpuinfo.h"
//#include "arch/x86/cpuinfo.h"
#include "kprintf.h"
#include <string.h>


#define CPUID_FUNC_VENDOR           0
#define CPUID_FUNC_FEATURES         1
#define CPUID_FUNC_TLB              2
#define CPUID_FUNC_SERIAL           3


/* CPUID detection flag. */
#define EFLAGS_ID   0x00200000


#if 0
/* Feature flags for Intel CPUs (edx register) */
static char *features0_str[] =
{
	"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
	"cx8", "apic", "rsvd", "sep", "mtrr", "pge", "mca", "cmov",
	"pat", "pse36", "psn", "clfsh", "rsvd", "ds", "acpi", "mmx",
	"fxsr", "sse", "sse2", "ss", "htt", "tm", "ia64", "pbe",
};

/* Feature flags for Intel CPUs (ecx register) */
static char *features1_str[] =
{
	"sse3", "pclmul", "dtes64", "monitor", "ds-cpl", "vmx", "smx", 
    "est", "tm2", "ssse3", "cid", "sdbg", "fma", "cx16", "etprd",
    "pdcm", "rsvd", "pcid", "dca", "sse4.1", "sse4.2", "x2apic", 
    "movbe", "popcnt", "tsc-ddl", "aes", "xsave", "osxsave", "avx", 
    "f16c", "rdrnd", "hypervisor"
};
#endif

/**
 * Get the CPUID information.
 * @param op The operation code to perform.
 * @param eax EAX register value after the CPUID execution.
 * @param ebx EBX register value after the CPUID execution.
 * @param ecx ECX register value after the CPUID execution.
 * @param edx EDX register value after the CPUID execution.
 * 
 * \warning
 *	Not all the Intel CPUs support the CPUID instruction!!!
 *	Only some Intel486 family and subsequent Intel processors
 *	provide this method for determinig the architecture flags.
 *	Execution of CPUID on a processor that does not support this
 *	instruction will result in an invalid opcode exception.
 *	\n \n
 *	To determine if it is possible to use this instruction we can
 *	use bit 21 of the EFLAGS register. If software can change the
 *	value of this flag, the CPUID instruction is executable.
 */
static inline void cpuid(int op, uint32_t *eax, uint32_t *ebx, 
            uint32_t *ecx, uint32_t *edx)
{
    /*
	asm volatile(
		"cpuid"
		: "=a" (*eax), "=b" (*ebx),
		  "=c" (*ecx), "=d" (*edx)
		: "a" (op)
	);
    */
}

static inline int cpuid_is_available(void)
{
    /*
	int flag1, flag2;
	asm volatile (
		"pushfl\n"
		"pushfl\n"
		"popl %0\n"
		"mov %0, %1\n"
		"xorl %2, %0\n"
		"pushl %0\n"
		"popfl\n"
		"pushfl\n"
		"popl %0\n"
		"popfl\n"
		: "=r"(flag1), "=r"(flag2) 
        : "i"(EFLAGS_ID)
	);
	return( (flag1^flag2) & EFLAGS_ID );
    */
    return 0;
}

struct cpuinfo_st cpuinfo;

void cpuinfo_init(void)
{
    uint32_t dummy;
    memset(&cpuinfo, 0, sizeof(cpuinfo));
    if (cpuid_is_available())
    {
        cpuid(CPUID_FUNC_VENDOR, &dummy, (uint32_t *)(cpuinfo.vendor),
              (uint32_t *)(cpuinfo.vendor+8), (uint32_t *)(cpuinfo.vendor+4));
        cpuinfo.vendor[12] = '\0';

		cpuid(CPUID_FUNC_FEATURES, &cpuinfo.signature, &cpuinfo.brand,
			  &cpuinfo.features[1], &cpuinfo.features[0]);
    }
}
