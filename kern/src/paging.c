#include "paging.h"

extern uint8_t _kernel_arena_start[];
extern uint8_t _kernel_arena_end[];

uint8_t *arena = _kernel_arena_start;
size_t size, offset;

static int
kern_alloc_validate(void)
{
	int ret = 0;
	if ((size < PAGE_SIZE) || ((size & 0xfff) != 0))
		return ret = KERN_ALLOC_ERROR_INVALID_SIZE;
	return ret;
}

int
kern_alloc_init(void)
{
	int ret = 0;
	size = (size_t)(_kernel_arena_end - _kernel_arena_start);
	offset = 0;
	return kern_alloc_validate();
}

int
kern_alloc_page(void **page)
{
	int ret = 0;
	if (offset + PAGE_SIZE > size)
		return ret = KERN_ALLOC_ERROR_OUT_OF_ARENA;
	*page = (void *)(arena + offset);
	offset += PAGE_SIZE;
	return ret;
}
