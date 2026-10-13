#ifndef _KERN_TSS_H
#define _KERN_TSS_H

#include "kern_types.h"

#define TSS_ENTRY_COUNT 26
#define TSS_SIZE (TSS_ENTRY_COUNT * 4)

#define TSS_RSP0_INDEX 1
#define TSS_RSP1_INDEX 3
#define TSS_RSP2_INDEX 5
#define TSS_IST1_INDEX 9
#define TSS_IST2_INDEX 11
#define TSS_IST3_INDEX 13
#define TSS_IST4_INDEX 15
#define TSS_IST5_INDEX 17
#define TSS_IST6_INDEX 19
#define TSS_IST7_INDEX 21
#define TSS_IOPB_INDEX 25

void tss_set_table(uint64_t);

void tss_modify_ptr_entry(uint32_t *, uint64_t, int);

void tss_modify_iopb_entry(uint32_t *, uint16_t);

extern uint32_t tss[];

#endif
