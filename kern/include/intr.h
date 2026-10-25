#ifndef _KERN_INTR_H
#define _KERN_INTR_H

#include "kern_types.h"

#define INTR_DE 0
#define INTR_DB 1
#define INTR_NMI 2
#define INTR_BP 3
#define INTR_OF 4
#define INTR_BR 5
#define INTR_UD 6
#define INTR_NM 7
#define INTR_DF 8
#define INTR_TS 10
#define INTR_NP 11
#define INTR_SS 12
#define INTR_GP 13
#define INTR_PF 14
#define INTR_MF 16
#define INTR_AC 17
#define INTR_MC 18
#define INTR_XM 19
#define INTR_VE 20
#define INTR_CP 21

struct intr_frame {
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t num;
	uint64_t err;
	uint64_t rip;
	uint64_t cs;
	uint64_t rfl;
	uint64_t rsp;
	uint64_t ss;
};

void intr_disable(void);

void intr_enable(void);

void intr_isr_glob(const struct intr_frame *);

int intr_get_isr_info(int, uint16_t, uint8_t, uint64_t *, uint16_t *,
    uint16_t *);

extern uint64_t isr_table[];

#endif
