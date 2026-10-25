#include "intr.h"

void
intr_isr_glob(uint64_t intr, uint64_t code)
{
	return;
}

int intr_get_isr_info(int intr, uint16_t gdti, uint8_t ist, uint64_t *isr,
    uint16_t *selector, uint16_t *flags)
{
	if (intr > 255)
		return KERN_ERROR_INVALID_INTR;
	if (ist > 7)
		return KERN_ERROR_INVALID_ISR_IST;
	*flags = (uint16_t)(ist | 0x8f00U);
	*selector = (uint16_t)(gdti & 0xfff8U);
	*isr = isr_table[intr];
	return 0;
}
