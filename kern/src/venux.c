#include "venux.h"

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

uint64_t kern_paddr = 0;
uint64_t kern_vaddr = 0;
uint64_t kern_size = 0;

uint64_t *phys_pml4 = NULL;

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
	return kern_map_addr_range(desc->phys_start, desc->virt_start,
	    desc->virt_start + desc->page_count * PAGE_SIZE, phys_pml4);
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
	if (ENTRY_PRESENT(phys_pml4[pml4i]))
		return ret = KERN_ERROR_PAGE_ALREADY_PRESENT;
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
