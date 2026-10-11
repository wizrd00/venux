#include "tss.h"

uint32_t tss[TSS_ENTRY_COUNT];

void
tss_modify_entry(uint32_t *tss, uint64_t addr, int index)
{
	tss[index] = (uint32_t)(addr & 0xffffffffULL);
	tss[index + 1] = (uint32_t)(addr >> 32);
	return;
}

void
tss_modify_rsp0(uint32_t *tss, uint64_t rsp0)
{
	tss_modify_entry(tss, rsp0, TSS_RSP0_INDEX);
	return;
}

void
tss_modify_rsp1(uint32_t *tss, uint64_t rsp1)
{
	tss_modify_entry(tss, rsp1, TSS_RSP1_INDEX);
	return;
}

void
tss_modify_rsp2(uint32_t *tss, uint64_t rsp2)
{
	tss_modify_entry(tss, rsp2, TSS_RSP2_INDEX);
	return;
}

void
tss_modify_ist1(uint32_t *tss, uint64_t ist1)
{
	tss_modify_entry(tss, ist1, TSS_IST1_INDEX);
	return;
}

void
tss_modify_ist2(uint32_t *tss, uint64_t ist2)
{
	tss_modify_entry(tss, ist2, TSS_IST2_INDEX);
	return;
}

void
tss_modify_ist3(uint32_t *tss, uint64_t ist3)
{
	tss_modify_entry(tss, ist3, TSS_IST3_INDEX);
	return;
}

void
tss_modify_ist4(uint32_t *tss, uint64_t ist4)
{
	tss_modify_entry(tss, ist4, TSS_IST4_INDEX);
	return;
}

void
tss_modify_ist5(uint32_t *tss, uint64_t ist5)
{
	tss_modify_entry(tss, ist5, TSS_IST5_INDEX);
	return;
}

void
tss_modify_ist6(uint32_t *tss, uint64_t ist6)
{
	tss_modify_entry(tss, ist6, TSS_IST6_INDEX);
	return;
}

void
tss_modify_ist7(uint32_t *tss, uint64_t ist7)
{
	tss_modify_entry(tss, ist7, TSS_IST7_INDEX);
	return;
}

void
tss_modify_iopb(uint32_t *tss, uint16_t iopb)
{
	tss[TSS_IOPB_INDEX] = (uint32_t)(iopb << 16);
	return;
}
