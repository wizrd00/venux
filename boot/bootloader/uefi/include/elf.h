#ifndef _BOOTLOADER_ELF_H
#define _BOOTLOADER_ELF_H

#include <Uefi.h>

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define EI_PAD 9

#define ET_EXEC 2

#define EM_X86_64 17

#define EV_CURRENT 1

#define MAGNUM {0x7f, 'E', 'L', 'F'}

#define ELFCLASS64 2

#define ELFDATA2LSB 1

#define ELFOSABI_SYSV 0

#define ELFNIDENT 16

#define PT_LOAD 1

struct boot_elf64_ehdr {
	UINT8 e_ident[ELFNIDENT];
	UINT16 e_type;
	UINT16 e_machine;
	UINT32 e_version;
	UINT64 e_entry;
	UINT64 e_phoff;
	UINT64 e_shoff;
	UINT32 e_flags;
	UINT16 e_ehsize;
	UINT16 e_phentsize;
	UINT16 e_phnum;
	UINT16 e_shentsize;
	UINT16 e_shnum;
	UINT16 e_shstrndx;
};

struct boot_elf64_phdr {
	UINT32 p_type;
	UINT32 p_flags;
	UINT64 p_offset;
	UINT64 p_vaddr;
	UINT64 p_paddr;
	UINT64 p_filesz;
	UINT64 p_memsz;
	UINT64 p_align;
};

#endif
