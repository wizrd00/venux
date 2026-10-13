#include "idt.h"

void
idt_set_gate(struct idt_gate *gate, uint64_t isr, uint16_t selector,
    uint16_t flags)

int
idt_modify_entry(struct idt_gate *idt, struct idt_gate *gate, int index)
{
	if (index > IDT_CP)
		return KERN_ERROR_INVALID_IDT_INDEX;
	idt[index] = *gate;
	return 0;
}
