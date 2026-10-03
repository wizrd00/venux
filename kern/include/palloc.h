#ifndef _KERN_PALLOC_H
#define _KERN_PALLOC_H

#include "kern_types.h"

int kern_init_arena(void);

void *kern_palloc(size_t);

extern uint8_t _kernel_arena_start[];
extern uint8_t _kernel_arena_end[];

extern uint8_t *arena;

#endif
