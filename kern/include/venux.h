#ifndef _KERN_VENUX_H
#define _KERN_VENUX_H

#include "kern_types.h"
#include "arena.h"
#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "intr.h"
#include "vmm.h"

#define KERNEL_CODE_ACCESS 0x9bU
#define KERNEL_CODE_FLAGS 0x0aU
#define KERNEL_CODE_LIMIT 0xfffffU
#define KERNEL_DATA_ACCESS 0x93U
#define KERNEL_DATA_FLAGS 0x08U
#define KERNEL_DATA_LIMIT 0xfffffU

#define USER_CODE_ACCESS 0xfbU
#define USER_CODE_FLAGS 0x0aU
#define USER_CODE_LIMIT 0xfffffU
#define USER_DATA_ACCESS 0xf3U
#define USER_DATA_FLAGS 0x08U
#define USER_DATA_LIMIT 0xfffffU

#define TSS_ACCESS 0x89U
#define TSS_FLAGS 0x0U

#define DEFAULT_IST_INDEX 0x1

#define EXTRACT_ADDR(_entry) (_entry & 0x000ffffffffff000ULL)

#define ALLOC_PML4(_pml4, _ret)\
	do {\
		_pml4 = (uint64_t *)kern_get_arena_pages(1);\
		if (_pml4 == NULL)\
			_ret = KERN_ERROR_OUT_OF_ARENA;\
	} while (0)

#define SET_GDT_CONF_KERNEL_CODE(_conf)\
	do {\
		_conf.base = 0ULL;\
		_conf.limit = KERNEL_CODE_LIMIT;\
		_conf.access = (uint8_t)KERNEL_CODE_ACCESS;\
		_conf.flags = (uint8_t)KERNEL_CODE_FLAGS;\
	} while (0)

#define SET_GDT_CONF_KERNEL_DATA(_conf)\
	do {\
		_conf.base = 0ULL;\
		_conf.limit = KERNEL_DATA_LIMIT;\
		_conf.access = (uint8_t)KERNEL_DATA_ACCESS;\
		_conf.flags = (uint8_t)KERNEL_DATA_FLAGS;\
	} while (0)

#define SET_GDT_CONF_USER_CODE(_conf)\
	do {\
		_conf.base = 0ULL;\
		_conf.limit = USER_CODE_LIMIT;\
		_conf.access = (uint8_t)USER_CODE_ACCESS;\
		_conf.flags = (uint8_t)USER_CODE_FLAGS;\
	} while (0)

#define SET_GDT_CONF_USER_DATA(_conf)\
	do {\
		_conf.base = 0ULL;\
		_conf.limit = USER_DATA_LIMIT;\
		_conf.access = (uint8_t)USER_DATA_ACCESS;\
		_conf.flags = (uint8_t)USER_DATA_FLAGS;\
	} while (0)

#define SET_GDT_CONF_TSS(_tss, _conf)\
	do {\
		_conf.base = (uint64_t)_tss;\
		_conf.limit = TSS_SIZE - 1;\
		_conf.access = (uint8_t)TSS_ACCESS;\
		_conf.flags = (uint8_t)TSS_FLAGS;\
	} while (0)

#define SET_IDT_INTR(_idt, _intr)\
	do {\
		ret = intr_get_isr_info(_intr,\
		(uint16_t)(GDT_KERNEL_CODE_INDEX << 3), DEFAULT_IST_INDEX,\
		&isr, &selector, &flags);\
		if (RET_ERROR(ret))\
			KERN_PANIC(ret);\
		idt_set_gate(&gate, isr, selector, flags);\
		ret = idt_modify_gate(_idt, &gate, _intr);\
		if (RET_ERROR(ret))\
			KERN_PANIC(ret);\
	} while (0)

void kern_main(struct kern_args *);

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];
extern uint8_t _kernel_text_start[];
extern uint8_t _kernel_text_end[];
extern uint8_t _kernel_rodata_start[];
extern uint8_t _kernel_rodata_end[];
extern uint8_t _kernel_data_start[];
extern uint8_t _kernel_data_end[];
extern uint8_t _kernel_bss_start[];
extern uint8_t _kernel_bss_end[];
extern uint8_t _kernel_arena_start[];
extern uint8_t _kernel_arena_end[];
extern uint8_t _kernel_stack_start[];
extern uint8_t _kernel_stack_end[];

extern uint64_t kern_paddr;
extern uint64_t kern_vaddr;
extern uint64_t kern_size;
extern uint64_t kern_text_vaddr;
extern uint64_t kern_text_size;
extern uint64_t kern_rodata_vaddr;
extern uint64_t kern_rodata_size;
extern uint64_t kern_data_vaddr;
extern uint64_t kern_data_size;
extern uint64_t kern_bss_vaddr;
extern uint64_t kern_bss_size;
extern uint64_t kern_arena_vaddr;
extern uint64_t kern_arena_size;
extern uint64_t kern_stack_vaddr;
extern uint64_t kern_stack_size;

extern uint64_t *phys_pml4;

extern uint64_t gdt[];
extern uint32_t tss[];
extern struct idt_gate idt[];

#endif
