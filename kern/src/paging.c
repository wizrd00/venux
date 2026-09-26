#include "paging.h"

static uint64_t *
convert_entry_into_table(uint64_t entry)
{
	if (((entry >> 47) & 1) == 1)
		entry |= 0xffff000000000000ULL;
	else
		entry &= 0x0000fffffffff000ULL;
	return (uint64_t *)CONVERT_KERNEL_PADDR(entry);
}

static int
kern_map_into_pt(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pt)
{
	int ret = 0;
	if (pt == NULL)
		return ret = KERN_ERROR_INVALID_PT;
	while (vaddr_s < vaddr_e) {
		int pti = (int)GET_PTI(vaddr_s);
		if (ENTRY_PRESENT(pt[pti]))
			return ret = KERN_ERROR_PAGE_ALREADY_PRESENT;
		pt[pti] = (paddr_s & 0xffffffffffffULL) | PTE_FLAGS;
		paddr_s += PAGE_SIZE;
		vaddr_s += PAGE_SIZE;
	}
	return ret;
}

static int
kern_map_into_pd(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pd)
{
	int ret = 0;
	if (pd == NULL)
		return ret = KERN_ERROR_INVALID_PD;
	while (vaddr_s < vaddr_e) {
		int pdi = (int)GET_PDI(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x1fffffULL) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x1fffffULL) + 1;
		if (HUGE_PAGE_ALIGNED(paddr_s) && HUGE_PAGE_ALIGNED(vaddr_s) &&
		    HUGE_PAGE_ALIGNED(bound_e)) {
			if (ENTRY_PRESENT(pd[pdi]))
				return ret = KERN_ERROR_PAGE_ALREADY_PRESENT;
			pd[pdi] = (paddr_s & 0xffffffffffffULL) | PDPSE_FLAGS;
		} else {
			if (!ENTRY_PRESENT(pd[pdi])) {
				ALLOC_PT(pd[pdi], ret, PDE_FLAGS);
				if (RET_ERROR(ret))
					return ret;
			}
			uint64_t *pt = convert_entry_into_table(pd[pdi]);
			ret = kern_map_into_pt(paddr_s, vaddr_s, bound_e, pt);
			if (RET_ERROR(ret))
				return ret;
		}
		paddr_s += bound_e - vaddr_s;
		vaddr_s += bound_e - vaddr_s;
	}
	return ret;
}

static int
kern_map_into_pdpt(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pdpt)
{
	int ret = 0;
	if (pdpt == NULL)
		return ret = KERN_ERROR_INVALID_PDPT;
	while (vaddr_s < vaddr_e) {
		int pdpti = (int)GET_PDPTI(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x3fffffffULL) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x3fffffff) + 1;
		if (!ENTRY_PRESENT(pdpt[pdpti])) {
			ALLOC_PD(pdpt[pdpti], ret, PDPTE_FLAGS);
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pd = convert_entry_into_table(pdpt[pdpti]);
		ret = kern_map_into_pd(paddr_s, vaddr_s, bound_e, pd);
		if (RET_ERROR(ret))
			return ret;
		paddr_s += bound_e - vaddr_s;
		vaddr_s += bound_e - vaddr_s;
	}
	return ret;
}

static int
kern_map_into_pml4(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pml4)
{
	int ret = 0;
	if (pml4 == NULL)
		return ret = KERN_ERROR_INVALID_PHYS_PML4;
	while (vaddr_s < vaddr_e) {
		int pml4i = (int)GET_PML4I(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x7fffffffffULL) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x7fffffffffULL) + 1;
		if (!ENTRY_PRESENT(pml4[pml4i])) {
			ALLOC_PDPT(pml4[pml4i], ret, PML4E_FLAGS);
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pdpt = convert_entry_into_table(pml4[pml4i]);
		ret = kern_map_into_pdpt(paddr_s, vaddr_s, bound_e, pdpt);
		if (RET_ERROR(ret))
			return ret;
		paddr_s += bound_e - vaddr_s;
		vaddr_s += bound_e - vaddr_s;
	}
	return ret;
}

int
kern_map_addr_range(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pml4)
{
	return kern_map_into_pml4(paddr_s, vaddr_s, vaddr_e, pml4);
}
