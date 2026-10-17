#include "tss.h"

void
tss_modify_ptr_entry(uint32_t *tss, uint64_t addr, int index)
{
	tss[index] = (uint32_t)(addr & 0xffffffffULL);
	tss[index + 1] = (uint32_t)(addr >> 32);
	return;
}

void
tss_modify_iopb_entry(uint32_t *tss, uint16_t iopb)
{
	tss[TSS_IOPB_INDEX] = ((uint32_t)iopb << 16);
	return;
}
