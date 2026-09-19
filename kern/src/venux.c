#include "venux.h"

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

uint64_t kern_paddr = 0;
uint64_t kern_vaddr = 0;
uint64_t kern_size = 0;

int bios;

uint64_t *phys_pml4 = NULL;
static uint64_t pml4e_flags = 0x001;
static uint64_t pdpte_flags = 0x001;
static uint64_t pde_flags = 0x001;
static uint64_t pte_flags = 0x001;

static void
set_entry_flags(uint32_t type)
{
	pml4e_flags = 0x003;
	pdpte_flags = 0x003;
	pde_flags = 0x003;
	switch (convert_memtype(bios, type)) {
	case AVAILABLE :
	case ACPI_RECLAIM :
	case ACPI_NVS :
		pte_flags = 0x003;
		break;
	case RESERVED :
	default :
		pte_flags = 0x000;
		break;
	}
	return;
}

static int
kern_map_into_pt(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t *pt)
{
	int ret = 0;
	while (vaddr_s < vaddr_e) {
		int pti = (int)GET_PTI(vaddr_s);
		if (ENTRY_PRESENT(pt[pti]))
			return ret = KERN_ERROR_PAGE_ALREADY_PRESENT;
		pt[pti] = paddr_s | pte_flags;
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
	while (vaddr_s < vaddr_e) {
		int pdi = (int)GET_PDI(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x1fffff) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x1fffff) + 1;
		if (!ENTRY_PRESENT(pd[pdi])) {
			ALLOC_PT(pd[pdi], ret, pde_flags);
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pt = (uint64_t *)(pd[pdi] & 0xfffffffffffff000);
		ret = kern_map_into_pt(paddr_s, vaddr_s, vaddr_e, pt);
		if (RET_ERROR(ret))
			return ret;
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
	while (vaddr_s < vaddr_e) {
		int pdpti = (int)GET_PDPTI(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x3fffffff) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x3fffffff) + 1;
		if (!ENTRY_PRESENT(pdpt[pdpti])) {
			ALLOC_PD(pdpt[pdpti], ret, pdpte_flags);
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pd = (uint64_t *)(pdpt[pdpti] & 0xfffffffffffff000);
		ret = kern_map_into_pd(paddr_s, vaddr_s, vaddr_e, pd);
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
	while (vaddr_s < vaddr_e) {
		int pml4i = (int)GET_PML4I(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x7fffffffff) + 1 > vaddr_e) ?
		    vaddr_e : (vaddr_s | 0x7fffffffff) + 1;
		if (!ENTRY_PRESENT(phys_pml4[pml4i])) {
			ALLOC_PDPT(phys_pml4[pml4i], ret, pml4e_flags);
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pdpt = (uint64_t *)(phys_pml4[pml4i] &
		    0xfffffffffffff000);
		ret = kern_map_into_pdpt(paddr_s, vaddr_s, vaddr_e, pdpt);
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
	set_entry_flags(desc->type);
	if (phys_pml4 == NULL) {
		ALLOC_PML4(phys_pml4, ret);
		if (RET_ERROR(ret))
			return ret;
	}
	return kern_map_into_pml4((uint64_t)desc->phys_start,
	    (uint64_t)desc->virt_start,
	    (uint64_t)desc->virt_start + desc->page_count * PAGE_SIZE);
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
kern_map_kernel(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e)
{
	int ret = 0;
	if (vaddr_s >= vaddr_e)
		return ret = KERN_ERROR_INVALID_KERNEL_RANGE;
	return kern_map_into_pml4(paddr_s, vaddr_s, vaddr_e);
}

void
kern_main(struct kern_args *kargs)
{
	int ret = 0;
	bios = kargs->bios;
	kern_vaddr = (uint64_t)_kernel_start;
	kern_paddr = (uint64_t)kargs->kern_start;
	kern_size = (uint64_t)_kernel_end - kern_vaddr;
	ret = kern_alloc_init();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_physmem(&kargs->mem);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_kernel(kern_paddr, kern_vaddr, kern_vaddr + kern_size);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	return;
}
