#ifndef _KERN_TYPES_H
#define _KERN_TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "kern_globals.h"

#define UEFI_BIOS 0
#define LEGACY_BIOS 1

struct fb_info {
	void *base;
};

struct mem_info {
	void *info;
	size_t size;
	size_t count;
};

struct kern_args {
	int bios;
	void *uefi_rt;
	void *acpi;
	struct fb_info fb;
	struct mem_info mem;
};

#endif
