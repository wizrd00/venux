#ifndef _KERN_PALLOC_H
#define _KERN_PALLOC_H

#include "kern_types.h"

#define KERN_ALLOC_ERROR_INVALID_SIZE 1
#define KERN_ALLOC_ERROR_OUT_OF_ARENA 2

int kern_alloc_init(void);
void *kern_palloc(size_t count);

extern uint8_t *arena;
extern size_t size;
extern size_t offset;

#endif
