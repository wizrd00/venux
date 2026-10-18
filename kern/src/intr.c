#include "intr.h"

int intr_get_isr_info(int intr, uint8_t ist, uint8_t gdti, uint64_t *isr,
    uint16_t *flags, uint16_t *selector)
{
	if (ist > 7)
		return KERN_ERROR_INVALID_ISR_IST;
	switch (intr) {
	default :
		return KERN_ERROR_INVALID_INTR;
	}
	return 0;
}
