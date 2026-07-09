#ifndef ELF_H
#define ELF_H

#include <stdint.h>

/* ELF Magic */
#define ELF_MAGIC 0x464C457F  /* "\x7fELF" little endian */

/* ELF Types */
#define ET_EXEC 2  /* Executable */

/* Program Header Types */
#define PT_LOAD 1  /* Loadable segment */

/* ELF Header */
struct elf_header {
    uint32_t magic;         /* 0x7F 'E' 'L' 'F' */
    uint8_t  bits;          /* 1 = 32-bit, 2 = 64-bit */
    uint8_t  endian;        /* 1 = little, 2 = big */
    uint8_t  elf_version;
    uint8_t  os_abi;
    uint8_t  padding[8];
    uint16_t type;          /* ET_EXEC = executable */
    uint16_t machine;       /* 3 = x86 */
    uint32_t version;
    uint32_t entry;         /* Entry point address */
    uint32_t ph_offset;     /* Program header table offset */
    uint32_t sh_offset;     /* Section header table offset */
    uint32_t flags;
    uint16_t header_size;
    uint16_t ph_entry_size; /* Size of one program header */
    uint16_t ph_count;      /* Number of program headers */
    uint16_t sh_entry_size;
    uint16_t sh_count;
    uint16_t sh_str_index;
} __attribute__((packed));

/* Program Header */
struct elf_program_header {
    uint32_t type;          /* PT_LOAD = loadable */
    uint32_t offset;        /* Offset in file */
    uint32_t vaddr;         /* Virtual address to load at */
    uint32_t paddr;         /* Physical address (ignored) */
    uint32_t file_size;     /* Size in file */
    uint32_t mem_size;      /* Size in memory (>= file_size, rest zeroed) */
    uint32_t flags;         /* Permissions */
    uint32_t align;         /* Alignment */
} __attribute__((packed));

int  elf_load(const char* filename);

#endif