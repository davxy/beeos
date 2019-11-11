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

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "kbd.h"
#include "vmem.h"
#include "util.h"
#include "mm/frame.h"
#include "paging.h"
#include "panic.h"
#include "driver/ramdisk.h"
#include "kmalloc.h"
#include "kprintf.h"
#include <string.h>
#include <stdint.h>

/**
 * Multiboot descriptor.
 * A pointer to an initialized descriptor is passed to the kernel entry
 * routine via the ebx register by a multiboot compliat boot loader.
 * Notes:
 * Memory quantities are valid just if flags[0] == 1.
 * Boot device is valid if flags[1] == 1.
 * Syms present if flags[4] or flags[5] is set.
 */
struct multiboot_info {
    uint32_t flags;         /**< Various flags (see multiboot specification) */
    uint32_t mem_lower;     /**< Memory in KB starting from 0x0 */
    uint32_t mem_upper;     /**< Memory in KB starting from 0x100000 */
    uint32_t boot_device;   /**< Device that contains the kernel image */
    uint32_t cmdline;       /**< Command line argument. */
    uint32_t mods_count;    /**< Number of loaded modules */
    uint32_t mods_addr;     /**< Module address array address */
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
};

#define ZONE_LOW_TOP        0x400000
#define MB_HIGH_MEM_START   0x100000

/*
 * Moltiboot low mem zone (The first 1MB).
 * Instead of try to find out what parts of the low memory are
 * effectively usable just discard the first 1MB of memory.
 *
 * For the Kernel, the first 4MB of ram are classified as LOW memory.
 */
static void mm_init(const struct multiboot_info *mbi)
{
    char *addr, *end;
    size_t msize;
    int ret;

    msize = ALIGN_DOWN(mbi->mem_upper * 1024, PAGE_SIZE);
    /* At most add 3MB */
    if (msize > ZONE_LOW_TOP - MB_HIGH_MEM_START)
        msize = ZONE_LOW_TOP - MB_HIGH_MEM_START;
    ret = frame_zone_add((char *)MB_HIGH_MEM_START, msize, PAGE_SIZE, ZONE_LOW);
    if (ret < 0)
        panic("error adding low mem zone");

    /* Hack to get the kernel brk */
    addr = (char *)ALIGN_UP((uintptr_t)kmalloc(0,0), PAGE_SIZE);
    addr = (char *)virt_to_phys(addr);
    end = (char *)MB_HIGH_MEM_START + msize;
    /* Free unused space (after the kernel brk) */
    while (addr < end) {
        frame_free(addr, 0);
        addr += PAGE_SIZE;
    }
}

#define RESV 0x4000000

static void mm_high_init(const struct multiboot_info *mbi)
{
    char *addr, *end;
    size_t msize;
    int ret;

    msize = ALIGN_DOWN(mbi->mem_upper * 1024, PAGE_SIZE);
    /* Check if there is memory over the first 4MB */
    if (msize <= ZONE_LOW_TOP - MB_HIGH_MEM_START)
        return;
    msize -= (ZONE_LOW_TOP - MB_HIGH_MEM_START);

    /* Free HIGH zone memory (above ZONE_LOW_TOP) */
    addr = (char *)ZONE_LOW_TOP;
    end = (char *)ZONE_LOW_TOP + msize;

    ret = frame_zone_add((char *)ZONE_LOW_TOP, msize, PAGE_SIZE, ZONE_HIGH);
    if (ret < 0 && msize > RESV) {
        ret = frame_zone_add((char *)ZONE_LOW_TOP, RESV, PAGE_SIZE, ZONE_HIGH);
        if (ret == 0) {
            addr = (char *)ZONE_LOW_TOP;
            end = (char *)ZONE_LOW_TOP + RESV;
            while (addr < end) {
                frame_free(addr, 0);
                addr += PAGE_SIZE;
            }
            end = (char *)ZONE_LOW_TOP + msize;
            ret = frame_zone_add((char *)ZONE_LOW_TOP + RESV,
                    msize - RESV, PAGE_SIZE, ZONE_HIGH);
        }
    }
    if (ret < 0)
        panic("Error adding high mem zone");

    while (addr < end) {
        frame_free(addr, 0);
        addr += PAGE_SIZE;
    }
}

static void mod_load(const struct multiboot_info *mbi)
{
    char *addr, *start, *end;
    char **mods_addr = (char **)mbi->mods_addr;
    size_t size;

    start = mods_addr[0];
    end   = mods_addr[1];
    if (end < start)
        panic("malformed data within multiboot info");
    size = end - start;
    addr = (char *)kmalloc(size, 0);
    if (addr == NULL)
        panic("no space for initrd");
    memmove(addr, phys_to_virt(start), size); /* Takes care of overlaps */
    ramdisk_init(addr, size); /* Initialize ramdisk device */
}

const struct multiboot_info *g_mbi;

/*
 * Architecture specific initialization.
 * Must be executed before other generic routines.
 */
void arch_init(const struct multiboot_info *mbi)
{
    g_mbi = mbi;

    /*
     * Check for initrd.
     * To avoid corruption of the initrd content, this should be done
     * before any use of the memory allocator.
     */
    if (mbi->mods_count > 0)
        mod_load(mbi);

    /* Initialize global descriptor table */
    gdt_init();

    /* Initialize interrupt descriptor table */
    idt_init();

    /* Initialize PIC conroller */
    pic_init();

    /* Initialize the kernel memory allocator */
    mm_init(mbi);

    /* Finish with paging initialization */
    paging_init();


}

void arch_final(void)
{
    mm_high_init(g_mbi);

    /* Initialize keyboard */
    kbd_init();
}
