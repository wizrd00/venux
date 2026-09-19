#ifndef _KERN_PALLOC_H
#define _KERN_PALLOC_H

#include "kern_types.h"

int kern_alloc_init(void);
void *kern_palloc(size_t count);

extern uint8_t *arena;
extern size_t size;
extern size_t offset;

#endif
