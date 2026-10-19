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

void intr_isr_de(void);

void intr_isr_db(void);

void intr_isr_nmi(void);

void intr_isr_bp(void);

void intr_isr_of(void);

void intr_isr_br(void);

void intr_isr_ud(void);

void intr_isr_nm(void);

void intr_isr_df(void);

void intr_isr_ts(void);

void intr_isr_np(void);

void intr_isr_ss(void);

void intr_isr_gp(void);

void intr_isr_pf(void);

void intr_isr_mf(void);

void intr_isr_ac(void);

void intr_isr_mc(void);

void intr_isr_xm(void);

void intr_isr_ve(void);

void intr_isr_cp(void);

void intr_isr_glob(uint64_t, uint64_t);

int intr_get_isr_info(int, uint16_t, uint8_t, uint64_t *, uint16_t *,
    uint16_t *);

#endif
