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

#ifndef _BEEOS_ARCH_X86_CPUINFO_H_
#define _BEEOS_ARCH_X86_CPUINFO_H_

/**
 * @{ CPU features
 */
#define CPUINFO_FEAT0_FPU           (1 << 0)  
#define CPUINFO_FEAT0_VME           (1 << 1)  
#define CPUINFO_FEAT0_DE            (1 << 2)  
#define CPUINFO_FEAT0_PSE           (1 << 3)  
#define CPUINFO_FEAT0_TSC           (1 << 4)  
#define CPUINFO_FEAT0_MSR           (1 << 5)  
#define CPUINFO_FEAT0_PAE           (1 << 6)  
#define CPUINFO_FEAT0_MCE           (1 << 7)
#define CPUINFO_FEAT0_CX8           (1 << 8)
#define CPUINFO_FEAT0_APIC          (1 << 9)
#define CPUINFO_FEAT0_SEP           (1 << 11) 
#define CPUINFO_FEAT0_MTRR          (1 << 12) 
#define CPUINFO_FEAT0_PGE           (1 << 13) 
#define CPUINFO_FEAT0_MCA           (1 << 14) 
#define CPUINFO_FEAT0_CMOV          (1 << 15) 
#define CPUINFO_FEAT0_PAT           (1 << 16) 
#define CPUINFO_FEAT0_PSE36         (1 << 17) 
#define CPUINFO_FEAT0_PSN           (1 << 18) 
#define CPUINFO_FEAT0_CLFSH         (1 << 19) 
#define CPUINFO_FEAT0_DS            (1 << 21) 
#define CPUINFO_FEAT0_ACPI          (1 << 22) 
#define CPUINFO_FEAT0_MMX           (1 << 23) 
#define CPUINFO_FEAT0_FXSR          (1 << 24) 
#define CPUINFO_FEAT0_SSE           (1 << 25) 
#define CPUINFO_FEAT0_SSE2          (1 << 26) 
#define CPUINFO_FEAT0_SS            (1 << 27) 
#define CPUINFO_FEAT0_HTT           (1 << 28) 
#define CPUINFO_FEAT0_TM            (1 << 29) 
#define CPUINFO_FEAT0_IA64          (1 << 30)
#define CPUINFO_FEAT0_PBE           (1 << 31)
#define CPUINFO_FEAT1_SSE3          (1 << 0) 
#define CPUINFO_FEAT1_PCLMUL        (1 << 1)
#define CPUINFO_FEAT1_DTES64        (1 << 2)
#define CPUINFO_FEAT1_MONITOR       (1 << 3)  
#define CPUINFO_FEAT1_DS_CPL        (1 << 4)  
#define CPUINFO_FEAT1_VMX           (1 << 5)  
#define CPUINFO_FEAT1_SMX           (1 << 6)  
#define CPUINFO_FEAT1_EST           (1 << 7)  
#define CPUINFO_FEAT1_TM2           (1 << 8)  
#define CPUINFO_FEAT1_SSSE3         (1 << 9)  
#define CPUINFO_FEAT1_CID           (1 << 10)
#define CPUINFO_FEAT1_SDBG          (1 << 11)
#define CPUINFO_FEAT1_FMA           (1 << 12)
#define CPUINFO_FEAT1_CX16          (1 << 13) 
#define CPUINFO_FEAT1_ETPRD         (1 << 14) 
#define CPUINFO_FEAT1_PDCM          (1 << 15) 
#define CPUINFO_FEAT1_PCID          (1 << 17)
#define CPUINFO_FEAT1_DCA           (1 << 18) 
#define CPUINFO_FEAT1_SSE4_1        (1 << 19) 
#define CPUINFO_FEAT1_SSE4_2        (1 << 20) 
#define CPUINFO_FEAT1_X2APIC        (1 << 21) 
#define CPUINFO_FEAT1_MOVBE         (1 << 22) 
#define CPUINFO_FEAT1_POPCNT        (1 << 23) 
#define CPUINFO_FEAT1_TSC_DDL       (1 << 24)
#define CPUINFO_FEAT1_AES           (1 << 25) 
#define CPUINFO_FEAT1_XSAVE         (1 << 26) 
#define CPUINFO_FEAT1_OSXSAVE       (1 << 27) 
#define CPUINFO_FEAT1_AVX           (1 << 28)
#define CPUINFO_FEAT1_F16C          (1 << 29)
#define CPUINFO_FEAT1_RDRND         (1 << 30)
#define CPUINFO_FEAR1_HYPERVISOR    (1 << 31)
/** @} */


#endif /* _BEEOS_ARCH_X86_CPUINFO_H_ */
