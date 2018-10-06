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

#ifndef BEEOS_ELF_H_
#define BEEOS_ELF_H_

#include <stdint.h>

#define ELF_MAGIC               0x464C457FU


/* Legal values for ST_BIND subfield of st_info (symbol binding).  */

#define ELF_STB_LOCAL           0   /* Local symbol */
#define ELF_STB_GLOBAL          1   /* Global symbol */
#define ELF_STB_WEAK            2   /* Weak symbol */

/* Legal values for ST_TYPE subfield of st_info (symbol type).  */

#define ELF_STT_NOTYPE          0   /* Symbol type is unspecified */
#define ELF_STT_OBJECT          1   /* Symbol is a data object */
#define ELF_STT_FUNC            2   /* Symbol is a code object */


/** ELF file header. */
struct elf_hdr {
    uint32_t magic;         /**< Magic number. */
    uint8_t  elf[12];       /**< Other(?) information. */
    uint16_t type;          /**< Object file type. */
    uint16_t machine;       /**< Architecture. */
    uint32_t version;       /**< Object file version. */
    uint32_t entry;         /**< Entry point virtual address. */
    uint32_t phoff;         /**< Program header table file offset. */
    uint32_t shoff;         /**< Section header table file offset. */
    uint32_t flags;         /**< Processor-specific flags. */
    uint16_t ehsize;        /**< ELF header size in bytes. */
    uint16_t phentsize;     /**< Program header table entry size. */
    uint16_t phnum;         /**< Program header table entry count. */
    uint16_t shentsize;     /**< Section header table entry size. */
    uint16_t shnum;         /**< Section header table entry count. */
    uint16_t shstrndx;      /**< Section header strin table index. */
};


/* Legal value for type if segment type. */
#define ELF_PROG_TYPE_NULL      0   /**< Program entry unused. */
#define ELF_PROG_TYPE_LOAD      1   /**< Loadable program segment. */

/* Flag bits from program header flags */
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4


/** ELF programe segment header. */
struct elf_prog_hdr {
    uint32_t type;          /**< Segment type. */
    uint32_t offset;        /**< Segment file offset. */
    uint32_t vaddr;         /**< Segment virtual address. */
    uint32_t paddr;         /**< Segment physical address. */
    uint32_t filesz;        /**< Segment size in file. */
    uint32_t memsz;         /**< Segment size in memory. */
    uint32_t flags;         /**< Segment flags. */
    uint32_t align;         /**< Segment alignment. */
};

#define ELF_SECT_TYPE_NOBITS    8   /**< Not present in file (bss) */

/** ELF section header */
struct elf_section_hdr {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entsize;
};

struct elf_symbol_hdr {
    uint32_t name;
    uint32_t value;
    uint32_t size;
    uint8_t  info;
    uint8_t  other;
    uint16_t shndx;
};

struct elf_file {
    struct elf_symbol_hdr *symtab;
    uint32_t              symtabsz;
    const char            *strtab;
    uint32_t              strtabsz;
};

const char *elf_lookup_symbol(const struct elf_file *elf, uint32_t addr);


#endif /* BEEOS_ELF_H_ */
