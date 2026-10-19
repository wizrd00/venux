#include "intr.h"

static void
intr_isr_de(void)
{
	return;
}

static void
intr_isr_db(void)
{
	return;
}

static void
intr_isr_nmi(void)
{
	return;
}

static void
intr_isr_bp(void)
{
	return;
}

static void
intr_isr_of(void)
{
	return;
}

static void
intr_isr_br(void)
{
	return;
}

static void
intr_isr_ud(void)
{
	return;
}

static void
intr_isr_nm(void)
{
	return;
}

static void
intr_isr_df(void)
{
	return;
}

static void
intr_isr_ts(void)
{
	return;
}

static void
intr_isr_np(void)
{
	return;
}

static void
intr_isr_ss(void)
{
	return;
}

static void
intr_isr_gp(void)
{
	return;
}

static void
intr_isr_pf(void)
{
	return;
}

static void
intr_isr_mf(void)
{
	return;
}

static void
intr_isr_ac(void)
{
	return;
}

static void
intr_isr_mc(void)
{
	return;
}

static void
intr_isr_xm(void)
{
	return;
}

static void
intr_isr_ve(void)
{
	return;
}

static void
intr_isr_cp(void)
{
	return;
}

int intr_get_isr_info(int intr, uint16_t gdti, uint8_t ist, uint64_t *isr,
    uint16_t *selector, uint16_t *flags)
{
	if (ist > 7)
		return KERN_ERROR_INVALID_ISR_IST;
	*flags = (uint16_t)(ist | 0x8f00U);
	*selector = (uint16_t)(gdti & 0xfff8U);
	switch (intr) {
	case INTR_DE :
		*isr = (uint64_t)intr_isr_de;
		break;
	case INTR_DB :
		*isr = (uint64_t)intr_isr_db;
		break;
	case INTR_NMI :
		*isr = (uint64_t)intr_isr_nmi;
		break;
	case INTR_BP :
		*isr = (uint64_t)intr_isr_bp;
		break;
	case INTR_OF :
		*isr = (uint64_t)intr_isr_of;
		break;
	case INTR_BR :
		*isr = (uint64_t)intr_isr_br;
		break;
	case INTR_UD :
		*isr = (uint64_t)intr_isr_ud;
		break;
	case INTR_NM :
		*isr = (uint64_t)intr_isr_nm;
		break;
	case INTR_DF :
		*isr = (uint64_t)intr_isr_df;
		break;
	case INTR_TS :
		*isr = (uint64_t)intr_isr_ts;
		break;
	case INTR_NP :
		*isr = (uint64_t)intr_isr_np;
		break;
	case INTR_SS :
		*isr = (uint64_t)intr_isr_ss;
		break;
	case INTR_GP :
		*isr = (uint64_t)intr_isr_gp;
		break;
	case INTR_PF :
		*isr = (uint64_t)intr_isr_pf;
		break;
	case INTR_MF :
		*isr = (uint64_t)intr_isr_mf;
		break;
	case INTR_AC :
		*isr = (uint64_t)intr_isr_ac;
		break;
	case INTR_MC :
		*isr = (uint64_t)intr_isr_mc;
		break;
	case INTR_XM :
		*isr = (uint64_t)intr_isr_xm;
		break;
	case INTR_VE :
		*isr = (uint64_t)intr_isr_ve;
		break;
	case INTR_CP :
		*isr = (uint64_t)intr_isr_cp;
		break;
	default :
		return KERN_ERROR_INVALID_INTR;
	}
	return 0;
}
