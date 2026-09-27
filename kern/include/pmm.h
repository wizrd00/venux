#ifndef _KERN_PMM_H
#define _KERN_PMM_H

#include "kern_types.h"
#include "palloc.h"

#define MEMTYPE_UNKNOWN 0
#define MEMTYPE_AVAILABLE 1
#define MEMTYPE_RESERVED 2
#define MEMTYPE_BOOTLOADER 3
#define MEMTYPE_ACPI_RECLAIM 4
#define MEMTYPE_ACPI_NVS 5

#define PML4E_FLAGS 0x003ULL
#define PDPTE_FLAGS 0x003ULL
#define PDPSE_FLAGS 0x083ULL
#define PDE_FLAGS 0x003ULL
#define PTE_FLAGS 0x003ULL

#define CONVERT_KERNEL_VADDR(_addr) ((uint64_t)_addr - KERNEL_OFFSET)
#define CONVERT_KERNEL_PADDR(_addr) ((uint64_t)_addr + KERNEL_OFFSET)
#define CONVERT_PHYSMEM_VADDR(_addr) ((uint64_t)_addr - PHYSMEM_OFFSET)
#define CONVERT_PHYSMEM_PADDR(_addr) ((uint64_t)_addr + PHYSMEM_OFFSET)

#define ALLOC_PDPT(_entry, _ret, _flags)\
	do {\
		uint64_t *_tmp = (uint64_t *)kern_palloc(1);\
		if (_tmp == NULL){ \
			_ret = KERN_ERROR_OUT_OF_ARENA;\
			break;\
		}\
		_entry = (CONVERT_KERNEL_VADDR(_tmp) & 0xffffffffffffULL) |\
		    _flags;\
	} while (0)

#define ALLOC_PD(_entry, _ret, _flags)\
	do {ALLOC_PDPT(_entry, _ret, _flags);} while (0)

#define ALLOC_PT(_entry, _ret, _flags)\
	do {ALLOC_PDPT(_entry, _ret, _flags);} while (0)

#define ENTRY_PRESENT(_entry) ((_entry & 0x1ULL) == 1)

#define GET_PML4I(_addr) ((_addr >> 39) & 0x1ffULL)
#define GET_PDPTI(_addr) ((_addr >> 30) & 0x1ffULL)
#define GET_PDI(_addr) ((_addr >> 21) & 0x1ffULL)
#define GET_PTI(_addr) ((_addr >> 12) & 0x1ffULL)

void pmm_set_pml4(uint64_t *pml4);

void pmm_get_pml4(uint64_t **pml4);

int pmm_map_region(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pml4);

#endif
