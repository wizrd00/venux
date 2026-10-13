#ifndef _KERN_IDT_H
#define _KERN_IDT_H

#include "kern_types.h"

#define IDT_GATE_COUNT 256
#define IDT_SIZE (IDT_GATE_COUNT * 16)

#define IDT_DE 0
#define IDT_DB 1
#define IDT_NMI 2
#define IDT_BP 3
#define IDT_OF 4
#define IDT_BR 5
#define IDT_UD 6
#define IDT_NM 7
#define IDT_DF 8
#define IDT_TS 10
#define IDT_NP 11
#define IDT_SS 12
#define IDT_GP 13
#define IDT_PF 14
#define IDT_MF 16
#define IDT_AC 17
#define IDT_MC 18
#define IDT_XM 19
#define IDT_VE 20
#define IDT_CP 21

struct idt_gate {
	uint32_t offset0;
	uint16_t selector;
	uint16_t flags;
	uint16_t offset1;
	uint16_t offset2;
	uint32_t resv;
};

void idt_set_table(uint64_t, uint16_t);

void idt_set_gate(struct idt_gate *, uint64_t, uint16_t, uint16_t)

int idt_modify_gate(uint64_t *, struct idt_gate *, int);

#endif
