#ifndef _KERN_GDT_H
#define _KERN_GDT_H

#include "kern_types.h"

#define GDT_ENTRY_COUNT 7

#define GDT_NULL_INDEX 0
#define GDT_KERNEL_CODE_INDEX 1
#define GDT_KERNEL_DATA_INDEX 2
#define GDT_USER_DATA_INDEX 3
#define GDT_USER_CODE_INDEX 4
#define GDT_TSS_INDEX 5

struct gdt_entry_conf {
	uint64_t base;
	uint32_t limit;
	uint8_t access;
	uint8_t flags;
};

void gdt_set_table(uint64_t base, uint16_t limit);

int gdt_modify_kernel_code(struct gdt_entry_conf *);

int gdt_modify_kernel_data(struct gdt_entry_conf *);

int gdt_modify_user_code(struct gdt_entry_conf *);

int gdt_modify_user_data(struct gdt_entry_conf *);

int gdt_modify_tss(struct gdt_entry_conf *);

extern uint64_t gdt[];

#endif
