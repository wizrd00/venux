#ifndef _KERN_TYPES_H
#define _KERN_TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "kern_globals.h"

#define UEFI_BIOS 0
#define LEGACY_BIOS 1

struct mem_desc {
	uint32_t type;
	uint8_t *phys_start;
	uint8_t *virt_start;
	uint64_t page_count;
	uint64_t attr;
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
	void *uefi_rt;
	void *acpi;
	struct fb_info fb;
	struct mem_info mem;
};

#endif
