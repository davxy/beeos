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

/*
 * Recursive page directories are used.
 * The last page directory entry points to the page directory itself.
 */

#include "paging.h"
#include "vmem.h"
#include "isr.h"
#include "kprintf.h"
#include "mm/frame.h"
#include "panic.h"
#include "proc.h"
#include "sys.h"
#include <string.h>
#include <errno.h>

/*
 * Recursive page mapping allows to access to the page directory and tables
 * via the following special virtual addresses thus there is no need to
 * temporary map dirs and tables to a reserved virtual address.
 *
 * The last 4 MB of the current process virtual memory space
 * [0xFFC00000:0xFFFFFFFF] are reserved to access to the current process page
 * directory and page tables.
 * The second-to-last 4 MB of the current process virtual memory space
 * [0xFF800000:0xFFC00000) are reserved to eventually access to another task
 * directory and page tables.
 * Loosing 8 MB of memory space in 4 GB is not such a big deal.
 *
 * We also reserve a "wild" page starting below the second-to-last 4MB to
 * temporary map arbitrary physical addresses to a well known virtual address.
 * This wild page is used to copy pages between two different processes.
 */
#define PAGE_TAB_MAP    0xFFC00000  /* Current page tables base vaddress */
#define PAGE_DIR_MAP    0xFFFFF000  /* Current page directory vaddress */
#define PAGE_TAB_MAP2   0xFF800000  /* Temporary page tables base vaddress */
#define PAGE_WILD       (PAGE_TAB_MAP2-4096) /* Temporary "wild" page */

/* Virtual address to page directory index (virt / 4M) */
#define DIR_INDEX(virt) ((uint32_t)(virt) >> 22)
/* Virtual address to page table index (virt % 4M) / 4096 */
#define TAB_INDEX(virt) (((uint32_t)(virt) & 0x3FFFFF) >> 12)

/* Flush TLB */
#define flush_tlb() \
    asm volatile("mov eax, cr3\n\t" \
                 "mov cr3, eax\n\t" \
                  : : : "eax")

/*
 * TODO: is this an i386 feature? Or has been introduced later?
 * asm volatile("invlpg (%0)" : : "r"(phys) : "memory")
 */
#define page_invalidate(phys) flush_tlb()

/* Get page fault address */
#define fault_addr_get(virt) \
    asm volatile("mov %0, cr2" : "=r"(virt))


/*
 * Maps a page virtual memory address to a physical memory address.
 */
uint32_t page_map(void *virt, uint32_t phys)
{
    unsigned int di = DIR_INDEX(virt);
    unsigned int ti = TAB_INDEX(virt);
    uint32_t tab_phys;
    uint32_t pag_phys = phys;
    uint32_t *dir = (uint32_t *)PAGE_DIR_MAP;
    uint32_t *tab = (uint32_t *)(PAGE_TAB_MAP + (di * 0x1000));
    uint32_t flags = PTE_P | PTE_W;

    /* Check if is user space memory */
    if ((uint32_t)virt < KVBASE)
        flags |= PTE_U;

    /*
     * Check if the page table is present.
     * Note that is not required to be identity mappable.
     * TODO: Add ZONE_ANY flag?
     */
    if (!(dir[di] & PTE_P)) {
        /* page table not present */
        tab_phys = (uint32_t)frame_alloc(0, ZONE_LOW);
        if (tab_phys == 0)
            return (uint32_t)-ENOMEM;
        dir[di] = tab_phys | flags;
        /* Clean the new page table entries */
        memset(tab, 0, PAGE_SIZE);
    }

    /*
     * Check if the virtual address is already mapped.
     * This use the ZONE_HIGH by default...
     * TODO: add a zone flag to the function params?
     */
    if (!(tab[ti] & PTE_P)) {
        /* page not present */
        if ((int32_t)pag_phys == -1) {
            /* By default we map to high mem */
            pag_phys = (uint32_t)frame_alloc(0, ZONE_HIGH);
            if (pag_phys == 0)
                return (uint32_t)-ENOMEM;
        }
        tab[ti] = pag_phys | flags;
    } else if (!(tab[ti] & PTE_W)) {
        /* read only page (cow) */
        panic("COW not implemented yet");
    } else {
        panic("already mapped");
    }

    flush_tlb(); /* Is this really required? */
    return pag_phys;
}

/*
 * Unmap a virtual memory address.
 */
uint32_t page_unmap(void *virt, int retain)
{
    unsigned int i;
    unsigned int di = DIR_INDEX(virt);
    unsigned int ti = TAB_INDEX(virt);
    uint32_t *dir = (uint32_t *)PAGE_DIR_MAP;
    uint32_t *tab = (uint32_t *)(PAGE_TAB_MAP + (di * 0x1000));
    uint32_t tab_phys;
    uint32_t pag_phys = -1;

    if((dir[di] & PTE_P) != 0) {
        if ((tab[ti] & PTE_P) != 0) {
            pag_phys = (tab[ti] & PTE_MASK);
            tab[ti] = 0;
            page_invalidate(pag_phys);
            if (retain == 0)
                frame_free((void *)pag_phys, 0);
        }

        /* Check if that was the last page in the page table */
        for (i = 0; i < 1024; i++) {
            if ((tab[i] & PTE_P) != 0)
                break;
        }

        if (i == 1024) { /* If is the last page, delete the page table */
            tab_phys = (PTE_MASK & dir[di]);
            dir[di] = 0;
            page_invalidate(tab_phys);
            frame_free((void *)tab_phys, 0);
        }
    }
    flush_tlb();
    return pag_phys;
}

/*
 * Delete a page directory.
 */
void page_dir_del(uint32_t phys)
{
    unsigned int di, ti;
    const uint32_t *tab;
    const uint32_t *dir;
    uint32_t *dir_curr;

    dir_curr = (uint32_t *)PAGE_DIR_MAP;
    /* Temporary map the dir in under the current dir */
    dir_curr[1022] = phys | PTE_W | PTE_P;
    dir = (uint32_t *)(PAGE_TAB_MAP + (1022 * 4096));

    /*
     * Release user space
     */
    for (di = 0; di < 768; di++) {
        if ((dir[di] & PTE_P) != 0) {
            tab = (uint32_t *)(PAGE_TAB_MAP2 + (di * 4096));
            for (ti = 0; ti < 1024; ti++) {
                if ((tab[ti] & PTE_P) != 0)
                    frame_free((char *)(tab[ti] & PTE_MASK), 0);
            }
            frame_free((char *)(dir[di] & PTE_MASK), 0);
        }
    }

    /* Finally free the dir frame */
    frame_free((char *)phys, 0);
    dir_curr[1022] = 0;
    flush_tlb();
}



static void page_tab_dup(uint32_t *dir_dst, unsigned int i, uint32_t flags)
{
    const uint32_t *tab_src;
    uint32_t *tab_dst;
    const void *mem_src;
    void *mem_dst;
    uint32_t phys;
    unsigned int j;

    tab_src = (uint32_t *)(PAGE_TAB_MAP + (i * PAGE_SIZE));
    tab_dst = (uint32_t *)(PAGE_TAB_MAP2 + (i * PAGE_SIZE));
    phys = page_map(tab_dst, -1);
    memset(tab_dst, 0, PAGE_SIZE);
    dir_dst[i] = phys | flags;

    for (j = 0; j < 1024; j++) {
        if (tab_src[j] != 0) {
            /* TODO: copy on write (in the page fault handler) */
            /*
             * tab_src[j] &= ~PTE_W; // NON SEMBRA FUNZIONARE...
             * tab_dst[j] = tab_src[j];
             */
            mem_src = (void *)((i * 0x400000) + (j * 0x1000));
            mem_dst = (void *)PAGE_WILD;
            phys = page_map(mem_dst, -1);
            memcpy(mem_dst, mem_src, PAGE_SIZE);
            if ((int)page_unmap(mem_dst, 1) < 0)
                panic("Unmapping a mapped page");
            tab_dst[j] = phys | flags;
        }
    }
}


/*
 * Duplicates the current process page directory.
 */
uint32_t page_dir_dup(int dup_user)
{
    unsigned int i;
    uint32_t *dir_src;
    uint32_t *dir_dst;
    uint32_t phys;
    uint32_t flags = PTE_W | PTE_P;

    dir_src = (uint32_t *)PAGE_DIR_MAP;
    dir_dst = (uint32_t *)(PAGE_TAB_MAP + (1022 * 4096));
    phys = (uint32_t) frame_alloc(0, 0);
    dir_src[1022] = (phys | flags); /* Temporary map the dst page table */
    memset(dir_dst, 0, PAGE_SIZE);

    /* Kernel code and data is shared */
    memcpy(&dir_dst[768], &dir_src[768], 254*4);
    dir_dst[1023] = phys | flags;
    dir_dst[1022] = 0;
    flush_tlb();

    if (dup_user != 0) {
        /* User space is deep copied */
        flags |= PTE_U;
        for (i = 0; i < 768; i++) {
            if (dir_src[i] != 0)
                page_tab_dup(dir_dst, i, flags);
        }
    }

    phys = (dir_src[1022] & PTE_MASK);
    dir_src[1022] = 0;
    page_invalidate(phys);
    return phys;
}

/*
 * Propagates a kernel virtual address mapping to all the other processes.
 * This happens for kmalloced virtual addresses that are after the initially
 * mapped kernel space (4MB).
 */
static void map_propagate(unsigned int idx)
{
    uint32_t *dir_src, *dir_dst;
    const struct task *other;

    other = list_container(current->tasks.next, struct task, tasks);
    /*
     * The non-current process page dir is mapped just below the
     * current process page directory.
     */
    dir_src = (uint32_t *)PAGE_DIR_MAP;
    dir_dst = (uint32_t *)(PAGE_TAB_MAP + (1022 * 4096));
    while (other != current) {
        dir_src[1022] = other->arch.pgdir | PTE_W | PTE_P;
        dir_dst[idx] = dir_src[idx];
        flush_tlb();
        other = list_container(other->tasks.next, struct task, tasks);
    }
    dir_src[1022] = 0;
    flush_tlb();
}

/* Page fault error bits */
/* The fault is caused by a page-protection violation. */
#define ERR_PRESENT (1 << 0)
/* The fault was caused by a write access */
#define ERR_WRITE   (1 << 1)
/* The fault was caused by user-mode code */
#define ERR_USER    (1 << 2)
/* The fault was triggered by an instruction fetch (only if NX bit is enabled)*/
#define ERR_FETCH   (1 << 4)

/*
 * Page fault interrupt handler.
 * Here, after some conditions checking, we try to resolve the fault
 * mapping a physical frame into the missing page.
 *
 * Mapping of kernel space pages are propagated in all the system
 * processes. This often happens during kernel heap expansion that
 * overflows in unmapped memory. Kernel heap must be consistent for
 * all the processes within the system.
 *
 * If the fault happens in user space (vaddr < KBASE) then we check that
 * the involved process have the rights to access to the required address.
 * If not we send a SEGV signal to the current process (TODO).
 * rdreference
 */
static void page_fault_handler(void)
{
    uint32_t virt, phys;
    int err, do_kill = 0;

    fault_addr_get(virt);
    err = current->arch.ifr->err_no;

#ifdef DEBUG_PAGING
    kprintf("pid: %d\n", current->pid);
    kprintf("page fault at 0x%x\n", current->arch.ifr->eip);
    kprintf("faulting address 0x%x\n", virt);
    kprintf("error code: %x\n", err);
    kprintf("--------\n");
#endif

    if ((err & (ERR_PRESENT | ERR_FETCH)) != 0) {
        kprintf("Protection fault or NX violation... kill process %d\n",
                current->pid);
        do_kill = 1;
    }
    if ((err & ERR_USER) != 0) {
        /*
         * User land process fault.
         * Send SIGSEGV in case that:
         * - Read/Write to KERNEL SPACE
         * - Can't expand stack (TODO)
         * - Can't expand heap (TODO)
         * - Accessing read only address for write (< heap_base) (TODO)
         */
        if (virt >= KVBASE)
            do_kill = 1;
    }

    phys = (uint32_t)frame_alloc(0, 0);
    if (phys == 0)
        panic("Out of mem in page fault handler");
    if ((int)page_map((char *)virt, (uint32_t)-1) < 0)
        panic("Map page error");

    if (virt >= KVBASE)
        map_propagate(DIR_INDEX(virt));

    if (do_kill)
        sys_kill(current->pid, SIGSEGV);
}

/*
 * Initialize paging subsystem.
 */
void paging_init(void)
{
    unsigned int i;
    uint32_t *tab, phys;

    /*
     * New page table physical address.
     * For the first process we preserve the page dir already in use.
     */
    phys = (uint32_t)frame_alloc(0, 0);

    /* Recursive page mapping trick */
    kpage_dir[1023] = (uint32_t)virt_to_phys(kpage_dir) | PTE_W | PTE_P;

    /* Temporary mapping to construct the page table */
    kpage_dir[0] = (phys | PTE_W | PTE_P);
    flush_tlb();

    tab = (uint32_t *)PAGE_TAB_MAP; /* Page table for virtual address 0x0 */
    for (i = 0; i < 1024; i++)      /* Identity map the first 4 MB */
        tab[i] = (i * PAGE_SIZE) | PTE_W | PTE_P;

    /*
     * Now the new kernel page table is ready to be used in place of the
     * current page dir entry. Note that this operation MUST be done after
     * the table construction (is flush strictly needed???)
     */
    kpage_dir[768] = kpage_dir[0];
    kpage_dir[0] = 0; /* Unmap the low 4MB */
    flush_tlb();

    /* Register the page fault handler */
    isr_register_handler(ISR_PAGE_FAULT, page_fault_handler);
}
