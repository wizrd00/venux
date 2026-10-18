#include "idt.h"

void
idt_set_gate(struct idt_gate *gate, uint64_t isr, uint16_t selector,
    uint16_t flags)
{
	gate->offset0 = (uint16_t)(isr & 0xffffULL);
	gate->offset1 = (uint16_t)((isr >> 16) & 0xffffULL);
	gate->offset2 = (uint16_t)(isr >> 32);
	gate->selector = selector;
	gate->flags = flags;
	gate->resv = 0;
	return;
}

int
idt_modify_gate(struct idt_gate *idt, const struct idt_gate *gate, int index)
{
	if (index >= IDT_GATE_COUNT)
		return KERN_ERROR_INVALID_IDT_INDEX;
	idt[index] = *gate;
	return 0;
}
