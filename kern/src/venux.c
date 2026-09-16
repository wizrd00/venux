#include "venux.h"

uint64_t *phys_pml4 = NULL;

static uint64_t
entry_flags(uint64_t attr)
{
	return attr;
}

static int
kern_map_physmem_pdpt(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t attr, uint64_t *pdpt)
{
	int ret = 0;
	while (vaddr_s < vaddr_e) {
		
	}
	return ret;
}

static int
kern_map_physmem_pml4(uint64_t paddr_s, uint64_t vaddr_s, uint64_t vaddr_e,
    uint64_t attr)
{
	int ret = 0;
	while (vaddr_s < vaddr_e) {
		int pml4i = (int)GET_PML4I(vaddr_s);
		uint64_t bound_e = ((vaddr_s | 0x7fffffffff) > vaddr_e) ?
		    vaddr_e : vaddr_s | 0x7fffffffff;
		if (!ENTRY_PRESENT(phys_pml4[pml4i])) {
			ALLOC_PDPT(phys_pml4[pml4i], ret, entry_flags(attr));
			if (RET_ERROR(ret))
				return ret;
		}
		uint64_t *pdpt = phys_pml4[pml4i] & 0xfffffffffffff000;
		ret = kern_map_physmem_pdpt(paddr_s, vaddr_s, vaddr_e, attr,
		    pdpt);
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
	if (phys_pml4 == NULL) {
		ALLOC_PML4(phys_pml4, ret);
		if (RET_ERROR(ret))
			return ret;
	}
	return kern_map_physmem_pml4((uint64_t)desc->phys_start,
	    (uint64_t)desc->virt_start,
	    (uint64_t)desc->virt_start + desc->page_count * PAGE_SIZE,
	    desc->attr);
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
}

void
kern_main(struct kern_args *kargs)
{
	int ret = 0;
	ret = kern_alloc_init();
	if (RET_ERROR(ret))
		KERN_CRITICAL_ERROR("kern_alloc_init() failed");
	ret = kern_map_physmem(&kargs->mem);
	return;
}
