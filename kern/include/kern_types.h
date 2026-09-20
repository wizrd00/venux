#ifndef _KERN_TYPES_H
#define _KERN_TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "kern_globals.h"
#include "kern_errors.h"
#include "kern_memtypes.h"

struct mem_desc {
	uint32_t type;
	uint64_t phys_start;
	uint64_t virt_start;
	uint64_t page_count;
};

struct fb_info {
	void *base;
};

struct mem_info {
	void *info;
	int size;
	int count;
};

struct kern_args {
	int bios;
	void *kern_start;
	void *uefi_rt;
	void *acpi;
	struct fb_info fb;
	struct mem_info mem;
};

#endif
