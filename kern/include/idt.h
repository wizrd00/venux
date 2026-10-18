#ifndef _KERN_IDT_H
#define _KERN_IDT_H

#include "kern_types.h"

#define IDT_GATE_COUNT 256
#define IDT_SIZE (IDT_GATE_COUNT * 16)

struct idt_gate {
	uint16_t offset0;
	uint16_t selector;
	uint16_t flags;
	uint16_t offset1;
	uint32_t offset2;
	uint32_t resv;
};

void idt_reload_table(uint64_t, uint16_t);

void idt_set_gate(struct idt_gate *, uint64_t, uint16_t, uint16_t);

int idt_modify_gate(struct idt_gate *, const struct idt_gate *, int);

#endif
