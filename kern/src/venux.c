#include "venux.h"

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

uint64_t kern_paddr = 0;
uint64_t kern_vaddr = 0;
uint64_t kern_size = 0;

uint64_t *phys_pml4 = NULL;

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
kern_map_into_pml4(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e)
{
	int ret = 0;
	if (phys_pml4 == NULL)
		return ret = KERN_ERROR_INVALID_PHYS_PML4;
	while (vaddr_s < vaddr_e) {
		int pml4i = (int)GET_PML4I(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x7fffffffffULL) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x7fffffffffULL) + 1;
		if (!ENTRY_PRESENT(phys_pml4[pml4i])) {
			ALLOC_PDPT(phys_pml4[pml4i], ret, PML4E_FLAGS);
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pdpt = convert_entry_into_table(phys_pml4[pml4i]);
		ret = kern_map_into_pdpt(paddr_s, vaddr_s, bound_e, pdpt);
		if (RET_ERROR(ret))
			return ret;
		paddr_s += bound_e - vaddr_s;
		vaddr_s += bound_e - vaddr_s;
	}
	return ret;
}

static int
kern_map_physmem_desc(struct mem_desc *desc)
{
	int ret = 0;
	if (desc->type == RESERVED)
		return ret;
	desc->virt_start = desc->phys_start + PHYSMEM_OFFSET;
	if (phys_pml4 == NULL) {
		return ret = KERN_ERROR_INVALID_PHYS_PML4;
	}
	return kern_map_into_pml4(desc->phys_start, desc->virt_start,
	    desc->virt_start + desc->page_count * PAGE_SIZE);
}

static int
kern_map_physmem(struct mem_info *mem)
{
	int ret = 0;
	if ((mem->size <= 0) || (mem->count <= 0))
		return ret = KERN_ERROR_INVALID_MEMMAP;
	uint8_t *ptr = (uint8_t *)mem->info;
	while (ptr < (uint8_t *)mem->info + (mem->count * mem->size)) {
		struct mem_desc *desc = (struct mem_desc *)ptr;
		ret = kern_map_physmem_desc(desc);
		if (RET_ERROR(ret))
			return ret;
		ptr += mem->size;
	}
	return ret;
}

static int
kern_map_kernel(void *pdpt)
{
	int ret = 0;
	if (phys_pml4 == NULL)
		return ret = KERN_ERROR_INVALID_PHYS_PML4;
	int pml4i = (int)((kern_vaddr >> 39) & 0x1ffULL);
	phys_pml4[pml4i] = ((uint64_t)pdpt & 0xffffffffffffULL) | PML4E_FLAGS;
	return ret;
}

static int
kern_init_phys_pml4(void)
{
	int ret = 0;
	if (phys_pml4 == NULL) {
		ALLOC_PML4(phys_pml4, ret);
		if (RET_ERROR(ret))
			return ret;
	}
	return ret;
}

void
kern_main(struct kern_args *kargs)
{
	int ret = 0;
	kern_vaddr = (uint64_t)_kernel_start;
	kern_paddr = (uint64_t)kargs->kern_start;
	kern_size = (uint64_t)_kernel_end - kern_vaddr;
	ret = kern_alloc_init();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_init_phys_pml4();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_kernel(kargs->kern_pdpt);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_physmem(&kargs->mem);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	kern_set_pml4((uint64_t *)CONVERT_KERNEL_VADDR(phys_pml4));
	return;
}
