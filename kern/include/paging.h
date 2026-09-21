#ifndef _KERN_PAGING_H
#define _KERN_PAGING_H

#include "kern_types.h"

void kern_set_pml4(uint64_t *pml4);

void kern_get_pml4(uint64_t **pml4);

#endif
