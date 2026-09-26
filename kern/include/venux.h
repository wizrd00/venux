#ifndef _KERN_VENUX_H
#define _KERN_VENUX_H

#include "kern_types.h"
#include "palloc.h"
#include "paging.h"

#define ALLOC_PML4(_pml4, _ret)\
	do {\
		_pml4 = (uint64_t *)kern_palloc(1);\
		if (_pml4 == NULL)\
			_ret = KERN_ERROR_OUT_OF_ARENA;\
	} while (0)

void kern_main(struct kern_args *);

extern uint64_t *phys_pml4;

#endif
