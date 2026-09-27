#include "venux.h"

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

struct kern_args args;
uint64_t kern_paddr = 0;
uint64_t kern_vaddr = 0;
uint64_t kern_size = 0;
uint64_t *phys_pml4 = NULL;

static int
kern_find_critical_memtypes(struct mem_info *mem)
{
	int ret = 0;
	int flags = 0;
	uint8_t *ptr = (uint8_t *)mem->info;
	while (ptr < (uint8_t *)mem->info + (mem->count * mem->size)) {
		struct mem_desc *desc = (struct mem_desc *)ptr;
		switch (desc->type) {
		case MEMTYPE_AVAILABLE :
			flags |= 0x1;
			break;
		case MEMTYPE_BOOTLOADER :
			flags |= 0x2;
			break;
		case MEMTYPE_ACPI_RECLAIM :
			flags |= 0x4;
			break;
		case MEMTYPE_ACPI_NVS :
			flags |= 0x8;
			break;
		default :
			break;
		}
		ptr += mem->size;
	}
	if ((flags & 0x1) == 0)
		return ret = KERN_ERROR_MISSING_AVAILABLE_MEMTYPE;
	if ((flags & 0x2) == 0)
		return ret = KERN_ERROR_MISSING_BOOTLOADER_MEMTYPE;
	if ((flags & 0x4) == 0)
		return ret = KERN_ERROR_MISSING_ACPI_RECLAIM_MEMTYPE;
	if ((flags & 0x8) == 0)
		return ret = KERN_ERROR_MISSING_ACPI_NVS_MEMTYPE;
	return ret;
}

static int
kern_map_kernel(void *pdpt)
{
	if (phys_pml4 == NULL)
		return KERN_ERROR_INVALID_PHYS_PML4;
	int pml4i = (int)((kern_vaddr >> 39) & 0x1ffULL);
	if (ENTRY_PRESENT(phys_pml4[pml4i]))
		return KERN_ERROR_PAGE_ALREADY_PRESENT;
	phys_pml4[pml4i] = ((uint64_t)pdpt & 0xffffffffffffULL) | PML4E_FLAGS;
	return 0;
}

static int
kern_map_desc(struct mem_desc *desc)
{
	desc->virt_start = CONVERT_PHYSMEM_PADDR(desc->phys_start);
	if (phys_pml4 == NULL) {
		return KERN_ERROR_INVALID_PHYS_PML4;
	}
	return pmm_map_region(desc->phys_start, desc->virt_start,
	    desc->virt_start + desc->page_count * PAGE_SIZE, phys_pml4);
}

static int
kern_map_avail_desc(struct mem_desc *desc)
{
	return (desc->type != MEMTYPE_AVAILABLE) ? 0 : kern_map_desc(desc);
}

static int
kern_map_bootldr_desc(struct mem_desc *desc)
{
	return (desc->type != MEMTYPE_BOOTLOADER) ? 0 : kern_map_desc(desc);
}

static int
kern_map_acpi_desc(struct mem_desc *desc)
{
	return ((desc->type != MEMTYPE_ACPI_RECLAIM) &&
	    (desc->type != MEMTYPE_ACPI_NVS)) ? 0 : kern_map_desc(desc);
}

static int
kern_map_physmem(struct mem_info *mem, int (*desc_mapper)(struct mem_desc *))
{
	int ret = 0;
	if ((mem->size <= 0) || (mem->count <= 0))
		return ret = KERN_ERROR_INVALID_MEMMAP;
	uint8_t *ptr = (uint8_t *)mem->info;
	while (ptr < (uint8_t *)mem->info + (mem->count * mem->size)) {
		struct mem_desc *desc = (struct mem_desc *)ptr;
		ret = desc_mapper(desc);
		if (RET_ERROR(ret))
			return ret;
		ptr += mem->size;
	}
	return ret;
}

static int
kern_init_pml4(void)
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
	args.kern_start = kargs->kern_start;
	args.kern_pdpt = kargs->kern_pdpt;
	ret = kern_arena_init();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_find_critical_memtypes(&kargs->mem);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_init_pml4();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_kernel(kargs->kern_pdpt);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_physmem(&kargs->mem, kern_map_avail_desc);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_physmem(&kargs->mem, kern_map_bootldr_desc);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_physmem(&kargs->mem, kern_map_acpi_desc);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	args.acpi = (void *)CONVERT_PHYSMEM_PADDR(kargs->acpi);
	pmm_set_pml4((uint64_t *)CONVERT_KERNEL_VADDR(phys_pml4));
	kargs = (struct kern_args *)CONVERT_PHYSMEM_PADDR(kargs);
	args.mem.info = (void *)CONVERT_PHYSMEM_PADDR(kargs->mem.info);
	args.mem.size = kargs->mem.size;
	args.mem.count = kargs->mem.count;
	return;
}
