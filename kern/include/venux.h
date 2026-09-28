#ifndef _KERN_VENUX_H
#define _KERN_VENUX_H

#include "kern_types.h"
#include "palloc.h"
#include "pmm.h"

#define ALLOC_PML4(_pml4, _ret)\
	do {\
		_pml4 = (uint64_t *)kern_palloc(1);\
		if (_pml4 == NULL)\
			_ret = KERN_ERROR_OUT_OF_ARENA;\
	} while (0)

void kern_main(struct kern_args *);

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];
extern uint8_t _kernel_text_start[];
extern uint8_t _kernel_text_end[];
extern uint8_t _kernel_rodata_start[];
extern uint8_t _kernel_rodata_end[];
extern uint8_t _kernel_data_start[];
extern uint8_t _kernel_data_end[];
extern uint8_t _kernel_bss_start[];
extern uint8_t _kernel_bss_end[];
extern uint8_t _kernel_arena_start[];
extern uint8_t _kernel_arena_end[];
extern uint8_t _kernel_stack_start[];
extern uint8_t _kernel_stack_end[];

extern uint64_t kern_paddr;
extern uint64_t kern_vaddr;
extern uint64_t *phys_pml4;

#endif
