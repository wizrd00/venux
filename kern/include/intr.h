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

void intr_disable(void);

void intr_enable(void);

int intr_get_isr_info(int, uint8_t, uint8_t, uint64_t *, uint16_t *,
    uint16_t *);

#endif
