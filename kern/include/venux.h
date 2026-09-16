#ifndef _KERN_VENUX_H
#define _KERN_VENUX_H

#include "kern_types.h"
#include "palloc.h"

#define KERN_ERROR_INVALID_MEMMAP 1
#define KERN_ERROR_PAGE_PRESENT 2

#define MEMORY_UC 0x0000000000000001UL
#define MEMORY_WC 0x0000000000000002UL
#define MEMORY_WT 0x0000000000000004UL
#define MEMORY_WB 0x0000000000000008UL
#define MEMORY_UCE 0x0000000000000010UL
#define MEMORY_WP 0x0000000000001000UL
#define MEMORY_RP 0x0000000000002000UL
#define MEMORY_XP 0x0000000000004000UL
#define MEMORY_NV 0x0000000000008000UL
#define MEMORY_MORE_RELIABLE 0x0000000000010000UL
#define MEMORY_RO 0x0000000000020000UL
#define MEMORY_SP 0x0000000000040000UL
#define MEMORY_CPU_CRYPTO 0x0000000000080000UL
#define MEMORY_HOT_PLUGGABLE 0x0000000000100000UL
#define MEMORY_RUNTIME 0x8000000000000000UL
#define MEMORY_ISA_VALID 0x4000000000000000UL
#define MEMORY_ISA_MASK 0x0FFFF00000000000UL

#define ALLOC_PML4(_pml4, _ret)\
	do {\
		_pml4 = (uint64_t *)kern_palloc(1);\
		if (_pml4 == NULL)\
			_ret = KERN_ALLOC_ERROR_OUT_OF_ARENA;\
	} while (0)

#define ALLOC_PDPT(_entry, _ret, _flags)\
	do {\
		uint64_t *_tmp = (uint64_t *)kern_palloc(1);\
		if (_tmp == NULL){ \
			_ret = KERN_ALLOC_ERROR_OUT_OF_ARENA;\
			break;\
		}\
		_entry = (uint64_t)_tmp | _flags;\
	} while (0)

#define ALLOC_PD(_entry, _ret, _flags)\
	do {ALLOC_PDPT(_entry, _ret, _flags);} while (0)

#define ALLOC_PT(_entry, _ret, _flags)\
	do {ALLOC_PDPT(_entry, _ret, _flags);} while (0)

#define ENTRY_PRESENT(_entry) ((_entry & 0x1) == 0)

#define GET_PML4I(_addr) ((_addr >> 39) & 0x1ff)
#define GET_PDPTI(_addr) ((_addr >> 30) & 0x1ff)
#define GET_PDI(_addr) ((_addr >> 21) & 0x1ff)
#define GET_PTI(_addr) ((_addr >> 12) & 0x1ff)

void kern_main(struct kern_args *);

extern uint64_t *phys_pml4;

#endif
