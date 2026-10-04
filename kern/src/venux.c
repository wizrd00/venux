#include "venux.h"

uint64_t kern_paddr;
uint64_t kern_vaddr;
uint64_t kern_size;
uint64_t *phys_pml4;
static uint64_t kern_text_vaddr;
static uint64_t kern_text_size;
static uint64_t kern_rodata_vaddr;
static uint64_t kern_rodata_size;
static uint64_t kern_data_vaddr;
static uint64_t kern_data_size;
static uint64_t kern_bss_vaddr;
static uint64_t kern_bss_size;
static uint64_t kern_arena_vaddr;
static uint64_t kern_arena_size;
static uint64_t kern_stack_vaddr;
static uint64_t kern_stack_size;

static uint64_t *
extract_kernel_vaddr(uint64_t entry)
{
	return (uint64_t *)CONVERT_KERNEL_P2V(EXTRACT_ADDR(entry));
}

static uint64_t *
extract_physmem_vaddr(uint64_t entry)
{
	return (uint64_t *)CONVERT_PHYSMEM_P2V(EXTRACT_ADDR(entry));
}

static int
kern_verify_memtypes(struct mem_info *mem)
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

static int
kern_map_kernel(void *pdpt)
{
	if (phys_pml4 == NULL)
		return KERN_ERROR_INVALID_PML4;
	int pml4i = (int)((kern_vaddr >> 39) & 0x1ffULL);
	if (ENTRY_PRESENT(phys_pml4[pml4i]))
		return KERN_ERROR_PAGE_ALREADY_PRESENT;
	phys_pml4[pml4i] = ((uint64_t)pdpt & 0xffffffffffffULL) | PML4E_FLAGS;
	return 0;
}

static int
kern_map_desc(struct mem_desc *desc)
{
	uint64_t vaddr_s = CONVERT_PHYSMEM_P2V(desc->phys_start);
	uint64_t vaddr_e = vaddr_s + desc->page_count * PAGE_SIZE;
	if (phys_pml4 == NULL) {
		return KERN_ERROR_INVALID_PML4;
	}
	return vmm_map_region(desc->phys_start, vaddr_s, vaddr_e , phys_pml4,
	    extract_kernel_vaddr);
}

static int
kern_map_avail_desc(struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_AVAILABLE) ? kern_map_desc(desc) : 0;
}

static int
kern_map_bootldr_desc(struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_BOOTLOADER) ? kern_map_desc(desc) : 0;
}

static int
kern_map_acpi_desc(struct mem_desc *desc)
{
	return ((desc->type == MEMTYPE_ACPI_RECLAIM) ||
	    (desc->type == MEMTYPE_ACPI_NVS)) ? kern_map_desc(desc) : 0;
}

static int
kern_iterate_memmap(struct mem_info *mem, int (*action)(struct mem_desc *))
{
	int ret = 0;
	if ((mem->size <= 0) || (mem->count <= 0))
		return ret = KERN_ERROR_INVALID_MEMMAP;
	uint8_t *ptr = (uint8_t *)mem->info;
	while (ptr < (uint8_t *)mem->info + (mem->count * mem->size)) {
		struct mem_desc *desc = (struct mem_desc *)ptr;
		ret = action(desc);
		if (RET_ERROR(ret))
			return ret;
		ptr += mem->size;
	}
	return ret;
}

static int
kern_map_physmem(struct mem_info *mem)
{
	int ret = 0;
	ret = kern_iterate_memmap(mem, kern_map_avail_desc);
	if (RET_ERROR(ret))
		return ret;
	ret = kern_iterate_memmap(mem, kern_map_bootldr_desc);
	if (RET_ERROR(ret))
		return ret;
	ret = kern_iterate_memmap(mem, kern_map_acpi_desc);
	if (RET_ERROR(ret))
		return ret;
	return ret;
}

static int
kern_secure_sections(void)
{
	int ret = 0;
	vmm_set_efer_nxe();
	kern_text_vaddr = (uint64_t)_kernel_text_start;
	kern_text_size = (uint64_t)(_kernel_text_end - _kernel_text_start);
	ret = vmm_set_permission(kern_text_vaddr,
	    kern_text_vaddr + kern_text_size, 0x1, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		return ret;
	kern_rodata_vaddr = (uint64_t)_kernel_rodata_start;
	kern_rodata_size = (uint64_t)(_kernel_rodata_end -
	    _kernel_rodata_start);
	ret = vmm_set_permission(kern_rodata_vaddr,
	    kern_rodata_vaddr + kern_rodata_size, 0x0, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		return ret;
	kern_data_vaddr = (uint64_t)_kernel_data_start;
	kern_data_size = (uint64_t)(_kernel_data_end - _kernel_data_start);
	ret = vmm_set_permission(kern_data_vaddr,
	    kern_data_vaddr + kern_data_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		return ret;
	kern_bss_vaddr = (uint64_t)_kernel_bss_start;
	kern_bss_size = (uint64_t)(_kernel_bss_end - _kernel_bss_start);
	ret = vmm_set_permission(kern_bss_vaddr,
	    kern_bss_vaddr + kern_bss_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		return ret;
	kern_arena_vaddr = (uint64_t)_kernel_arena_start;
	kern_arena_size = (uint64_t)(_kernel_arena_end - _kernel_arena_start);
	ret = vmm_set_permission(kern_arena_vaddr,
	    kern_arena_vaddr + kern_arena_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		return ret;
	kern_stack_vaddr = (uint64_t)_kernel_stack_start;
	kern_stack_size = (uint64_t)(_kernel_stack_end - _kernel_stack_start);
	ret = vmm_set_permission(kern_stack_vaddr,
	    kern_stack_vaddr + kern_stack_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		return ret;
	return ret;
}

static int
kern_disable_execperm(struct mem_desc *desc, uint8_t perm)
{
	uint64_t vaddr_s = CONVERT_PHYSMEM_P2V(desc->phys_start);
	uint64_t vaddr_e = vaddr_s + desc->page_count * PAGE_SIZE;
	return vmm_set_permission(vaddr_s, vaddr_e, perm, phys_pml4,
	    extract_physmem_vaddr);
}

static int
kern_disable_avail_execperm(struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_AVAILABLE) ?
	    kern_disable_execperm(desc, 0x2) : 0;
}

static int
kern_disable_bootldr_execperm(struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_BOOTLOADER) ?
	    kern_disable_execperm(desc, 0x2) : 0;
}

static int
kern_disable_acpi_execperm(struct mem_desc *desc)
{
	return ((desc->type == MEMTYPE_ACPI_RECLAIM) ||
	    (desc->type == MEMTYPE_ACPI_NVS)) ?
	    kern_disable_execperm(desc, 0x0) : 0;
}

static int
kern_secure_physmem(struct mem_info *mem)
{
	int ret = 0;
	ret = kern_iterate_memmap(mem, kern_disable_avail_execperm);
	if (RET_ERROR(ret))
		return ret;
	ret = kern_iterate_memmap(mem, kern_disable_bootldr_execperm);
	if (RET_ERROR(ret))
		return ret;
	ret = kern_iterate_memmap(mem, kern_disable_acpi_execperm);
	if (RET_ERROR(ret))
		return ret;
	return ret;
}

void
kern_main(struct kern_args *kargs)
{
	int ret = 0;
	kern_vaddr = (uint64_t)_kernel_start;
	kern_paddr = (uint64_t)kargs->kern_start;
	kern_size = (uint64_t)_kernel_end - kern_vaddr;
	ret = kern_init_arena();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_verify_memtypes(&kargs->mem);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_init_pml4();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_kernel(kargs->kern_pdpt);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_map_physmem(&kargs->mem);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	vmm_set_pml4((uint64_t *)CONVERT_KERNEL_V2P(phys_pml4));
	kargs = (struct kern_args *)CONVERT_PHYSMEM_P2V(kargs);
	kargs->kern_pdpt = (void *)CONVERT_PHYSMEM_P2V(kargs->kern_pdpt);
	kargs->acpi = (void *)CONVERT_PHYSMEM_P2V(kargs->acpi);
	kargs->mem.info = (void *)CONVERT_PHYSMEM_P2V(kargs->mem.info);
	ret = kern_secure_sections();
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_secure_physmem(&kargs->mem);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	return;
}
