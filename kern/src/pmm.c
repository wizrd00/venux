#include "pmm.h"

static int
pmm_map_into_pt(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
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
pmm_map_into_pd(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pd, uint64_t * (*extract_addr)(uint64_t))
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
			uint64_t *pt = extract_addr(pd[pdi]);
			ret = pmm_map_into_pt(paddr_s, vaddr_s, bound_e, pt);
			if (RET_ERROR(ret))
				return ret;
		}
		paddr_s += bound_e - vaddr_s;
		vaddr_s += bound_e - vaddr_s;
	}
	return ret;
}

static int
pmm_map_into_pdpt(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pdpt, uint64_t * (*extract_addr)(uint64_t))
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
		uint64_t *pd = extract_addr(pdpt[pdpti]);
		ret = pmm_map_into_pd(paddr_s, vaddr_s, bound_e, pd,
		    extract_addr);
		if (RET_ERROR(ret))
			return ret;
		paddr_s += bound_e - vaddr_s;
		vaddr_s += bound_e - vaddr_s;
	}
	return ret;
}

static int
pmm_map_into_pml4(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pml4, uint64_t * (*extract_addr)(uint64_t))
{
	int ret = 0;
	if (pml4 == NULL)
		return ret = KERN_ERROR_INVALID_PML4;
	while (vaddr_s < vaddr_e) {
		int pml4i = (int)GET_PML4I(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x7fffffffffULL) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x7fffffffffULL) + 1;
		if (!ENTRY_PRESENT(pml4[pml4i])) {
			ALLOC_PDPT(pml4[pml4i], ret, PML4E_FLAGS);
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pdpt = extract_addr(pml4[pml4i]);
		ret = pmm_map_into_pdpt(paddr_s, vaddr_s, bound_e, pdpt,
		    extract_addr);
		if (RET_ERROR(ret))
			return ret;
		paddr_s += bound_e - vaddr_s;
		vaddr_s += bound_e - vaddr_s;
	}
	return ret;
}

static int
pmm_set_entry_permission(uint64_t *entry, uint8_t perm)
{
	return 0;
}

static int
pmm_set_page_permission(uint64_t vaddr, uint64_t *psize, uint8_t perm,
    uint64_t *pml4, uint64_t * (*extract_addr)(uint64_t))
{
	int pml4i = (int)GET_PML4I(vaddr);
	if (!ENTRY_PRESENT(pml4[pml4i]))
		return KERN_ERROR_PML4E_NOT_PRESENT;
	uint64_t *pdpt = extract_addr(pml4[pml4i]);
	int pdpti = (int)GET_PDPTI(vaddr);
	if (!ENTRY_PRESENT(pdpt[pdpti]))
		return KERN_ERROR_PDPTE_NOT_PRESENT;
	uint64_t *pd = extract_addr(pdpt[pdpti]);
	int pdi = (int)GET_PDI(vaddr);
	if (!ENTRY_PRESENT(pd[pdi]))
		return KERN_ERROR_PDE_NOT_PRESENT;
	if (PDE_HUGE_PAGE(pd[pdi])) {
		pmm_set_entry_permission(pd + pdi, perm);
		*psize = HUGE_PAGE_SIZE;
		return 0;
	}
	uint64_t *pt = extract_addr(pd[pdi]);
	int pti = (int)GET_PTI(vaddr);
	if (!ENTRY_PRESENT(pt[pti]))
		return KERN_ERROR_PAGE_NOT_PRESENT;
	pmm_set_entry_permission(pt + pti, perm);
	*psize = PAGE_SIZE;
	return 0;
}

int
pmm_map_region(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pml4, uint64_t * (*extract_addr)(uint64_t))
{
	return pmm_map_into_pml4(paddr_s, vaddr_s, vaddr_e, pml4, extract_addr);
}

int
pmm_set_permission(uint64_t vaddr_s, uint64_t vaddr_e, uint8_t perm,
    uint64_t *pml4, uint64_t * (*extract_addr)(uint64_t))
{
	int ret = 0;
	if (pml4 == NULL)
		return ret = KERN_ERROR_INVALID_PML4;
	if ((!PAGE_ALIGNED(vaddr_s)) || (!PAGE_ALIGNED(vaddr_e)))
		return ret = KERN_ERROR_ADDR_NOT_PAGE_ALIGNED;
	if (vaddr_s == vaddr_e)
		return ret = KERN_ERROR_INVALID_REGION;
	if (!VALID_PERMISSION(perm))
		return ret = KERN_ERROR_INVALID_PERMISSION;
	while (vaddr_s < vaddr_e) {
		uint64_t psize;
		ret = pmm_set_page_permission(vaddr_s, &psize, perm, pml4,
		    extract_addr);
		if (RET_ERROR(ret))
			return ret;
		if (vaddr_s + psize > vaddr_e)
			return ret = KERN_ERROR_TOO_SMALL_REGION;
		vaddr_s += psize;
	}
	return ret;
}
