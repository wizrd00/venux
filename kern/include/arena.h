#ifndef _KERN_ARENA_H
#define _KERN_ARENA_H

#include "kern_types.h"

int kern_init_arena(void);

void *kern_get_page(size_t);

extern uint8_t _kernel_arena_start[];
extern uint8_t _kernel_arena_end[];

extern uint8_t *arena;

#endif
