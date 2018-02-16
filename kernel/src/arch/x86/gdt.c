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

#include "gdt.h"
#include "misc.h"
#include "paging.h"
#include <string.h>


/*
 * We just need two entries that defines the stack pointer and the stack
 * segment when we switch to kernel mode. All the other entries are unused
 */
struct tss_struct
{
    uint32_t prev;
    uint32_t esp0;  /* Stack pointer when we change to kernel mode */
    uint32_t ss0;   /* Stack segment when we change to kernel mode */
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
};

static struct gdt_entry     gdt_entries[6];
static struct gdt_register  gdt_reg;
struct tss_struct           tss;

/*
 * Load the new GDT register and flush the old one
 */
#define gdt_flush() asm volatile ( \
        "lgdt   [eax]\n\t" \
        "mov    eax, 0x10\n\t" \
        "mov    ds, ax\n\t" \
        "mov    es, ax\n\t" \
        "mov    fs, ax\n\t" \
        "mov    gs, ax\n\t" \
        "mov    ss, ax\n\t" \
        "jmp    0x08:1f\n\t" \
        "1:\n\t" \
    :: "a"(&gdt_reg))

/* Load task register */
#define load_task_reg() asm volatile( \
        "mov   ax, 0x2B \n\t" \
        "ltr   ax       \n\t")


/*
 * Initialize a single GDT entry
 */
static void gdt_entry_init(int i, uint32_t base, uint32_t limit,
        uint8_t flags, uint8_t access)
{
   gdt_entries[i].base_lo = (base & 0xFFFF);
   gdt_entries[i].base_mi = (base >> 16) & 0xFF;
   gdt_entries[i].base_hi = (base >> 24) & 0xFF;
   gdt_entries[i].limit_lo   = (limit & 0xFFFF);
   gdt_entries[i].flags = flags | ((limit >> 16) & 0x0F);
   gdt_entries[i].access = access;
}

/*
 * GDT initialization
 */
void gdt_init(void)
{
    uint32_t gdt_addr = (uint32_t)&gdt_entries;

    /* Init the GDT register */
    gdt_reg.limit = sizeof(struct gdt_entry) * 6 - 1;   /* Six entries */
    gdt_reg.base_lo = gdt_addr & 0xFFFF;
    gdt_reg.base_hi = (gdt_addr >> 16) & 0xFFFF;

    /*
     * Init the single entries.
     * common.flags  = (GR | SZ) = 0xC0
     * kcode.access  = (Pres | Dpl = 0 | Ex |  Rd) = 0x9A
     * kdata.access  = (Pres | Dpl = 0 | Wr) = 0x92
     * ucode.access  = (Pres | Dpl = 3 | Ex | Rd) = 0xFA
     * udata.access  = (Pres | Dpl = 3 | Wr) = 0xF2
     */
    memset(gdt_entries, 0, sizeof(struct gdt_entry));   /* NULL segment */
    gdt_entry_init(1, 0, 0xFFFFFFFF, 0xC0, 0x9A);       /* Kern code seg */
    gdt_entry_init(2, 0, 0xFFFFFFFF, 0xC0, 0x92);       /* Kern data seg */
    gdt_entry_init(3, 0, 0xFFFFFFFF, 0xC0, 0xFA);       /* User code seg */
    gdt_entry_init(4, 0, 0xFFFFFFFF, 0xC0, 0xF2);       /* User data seg */
    /*
     * TSS descriptor.
     * Requires the TSS address as 'base' and TSS size as 'limit'.
     * flags = SZ = 0x40
     * access = (Pres | Dpl = 3 | Ex | Ac) = 0xE9
     */
    gdt_entry_init(5, (uint32_t)&tss, sizeof(tss), 0x40, 0xE9);

    /* Make effective by loading the new GDT register */
    gdt_flush();

    /*
     * Initialize the Task State Segment descriptor.
     * Even though this structure is obsolete, some entries need to be
     * correctly initialized to control the values assumed by some
     * registers when the processor switch to privileged mode via the
     * 'int' instruction or after an irq.
     */
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;                             /* Kernel stack seg selector */
    tss.esp0 = (uint32_t)&kstack + PAGE_SIZE;   /* Kernel stack pointer */

    /* Load task register */
    load_task_reg();
}
