#include "arena.h"

uint8_t *arena = _kernel_arena_start;
static size_t size, offset;

static int
kern_validate_arena(void)
{
	int ret = 0;
	if ((size < PAGE_SIZE) || ((size & 0xfff) != 0))
		return ret = KERN_ERROR_INVALID_ARENA_SIZE;
	return ret;
}

int
kern_init_arena(void)
{
	int ret = 0;
	size = (size_t)(_kernel_arena_end - _kernel_arena_start);
	offset = 0;
	return kern_validate_arena();
}

void *
kern_get_page(size_t count)
{
	if (offset + (PAGE_SIZE * count) > size)
		return NULL;
	void *addr = (void *)(arena + offset);
	offset += PAGE_SIZE * count;
	return addr;
}
