/*
 * Copyright (c) 2015-2018, Davide Galassi. All rights reserved.
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

#ifndef BEEOS_ARCH_X86_PAGING_H_
#define BEEOS_ARCH_X86_PAGING_H_

#include "paging_bits.h"
#include "vmem.h"
#include <stdint.h>

/**
 * Duplicates the current process page directory.
 *
 * @param dup_user  Copy user space page tables.
 *                  Used by the execve syscall.
 * @return          New page directory physical address.
 */
uint32_t page_dir_dup(int dup_user);

/**
 * Deletes a page directory.
 *
 * @param phys  Physical address of the page directory.
 */
void page_dir_del(uint32_t phys);

/**
 * Maps a page virtual memory address to a physical memory address.
 *
 * @param virt  Page virtual memory address.
 * @param phys  Page physical memory address.
 *              If is -1, that is an invalid physical address, then the
 *              physical frame is allocated for us by the function.
 * @return      Page physical memory address.
 */
uint32_t page_map(void *virt, uint32_t phys);

/**
 * Unmaps a virtual memory address.
 *
 * @param virt      Page virtual memory address.
 * @param retain    If zero the page physical memory is freed, else
 *                  is retained and returned to the user.
 * @return          Page physical address, valid only if retained.
 */
uint32_t page_unmap(void *virt, int retain);

/**
 * Switch current page directory.
 *
 * @param phys  New page directory physical address
 */
#define page_dir_switch(phys) \
    asm volatile("mov cr3, %0\n\t" \
            : : "r"(phys))

/**
 * Initialize paging subsystem
 */
void paging_init(void);

#endif /* BEEOS_ARCH_X86_PAGING_H_ */
