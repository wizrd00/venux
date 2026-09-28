#ifndef _KERN_PALLOC_H
#define _KERN_PALLOC_H

#include "kern_types.h"

int kern_arena_init(void);

void *kern_palloc(size_t count);

extern uint8_t _kernel_arena_start[];
extern uint8_t _kernel_arena_end[];

extern uint8_t *arena;

#endif
