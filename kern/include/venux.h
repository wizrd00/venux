#ifndef _KERN_VENUX_H
#define _KERN_VENUX_H

#include "kern_types.h"
#include "palloc.h"
#include "paging.h"

#define PHYSMEM_OFFSET 0xffff800000000000ULL

#define PML4E_FLAGS 0x003ULL
#define PDPTE_FLAGS 0x003ULL

#define ALLOC_PML4(_pml4, _ret)\
	do {\
		_pml4 = (uint64_t *)kern_palloc(1);\
		if (_pml4 == NULL)\
			_ret = KERN_ERROR_OUT_OF_ARENA;\
	} while (0)

#define ALLOC_PDPT(_entry, _ret, _flags)\
	do {\
		uint64_t *_tmp = (uint64_t *)kern_palloc(1);\
		if (_tmp == NULL){ \
			_ret = KERN_ERROR_OUT_OF_ARENA;\
			break;\
		}\
		_entry = (uint64_t)_tmp | _flags;\
	} while (0)

#define ALLOC_PD(_entry, _ret, _flags)\
	do {ALLOC_PDPT(_entry, _ret, _flags);} while (0)

#define ALLOC_PT(_entry, _ret, _flags)\
	do {ALLOC_PDPT(_entry, _ret, _flags);} while (0)

#define ENTRY_PRESENT(_entry) ((_entry & 0x1) == 1)

#define GET_PML4I(_addr) ((_addr >> 39) & 0x1ffULL)
#define GET_PDPTI(_addr) ((_addr >> 30) & 0x1ffULL)
#define GET_PDI(_addr) ((_addr >> 21) & 0x1ffULL)
#define GET_PTI(_addr) ((_addr >> 12) & 0x1ffULL)

void kern_main(struct kern_args *);

extern int bios;
extern uint64_t *phys_pml4;

#endif
