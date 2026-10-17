#include "venux.h"

uint64_t kern_paddr;
uint64_t kern_vaddr;
uint64_t kern_size;
uint64_t *phys_pml4;
uint64_t gdt[GDT_ENTRY_COUNT];
uint32_t tss[TSS_ENTRY_COUNT];
uint64_t kern_text_vaddr;
uint64_t kern_text_size;
uint64_t kern_rodata_vaddr;
uint64_t kern_rodata_size;
uint64_t kern_data_vaddr;
uint64_t kern_data_size;
uint64_t kern_bss_vaddr;
uint64_t kern_bss_size;
uint64_t kern_arena_vaddr;
uint64_t kern_arena_size;
uint64_t kern_stack_vaddr;
uint64_t kern_stack_size;

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

static void
kern_init_addrs(const struct kern_args *kargs)
{
	kern_vaddr = (uint64_t)_kernel_start;
	kern_paddr = (uint64_t)kargs->kern_paddr;
	kern_size = (uint64_t)_kernel_end - kern_vaddr;
	kern_text_vaddr = (uint64_t)_kernel_text_start;
	kern_text_size = (uint64_t)(_kernel_text_end - _kernel_text_start);
	if (!PAGE_ALIGNED(kern_text_size))
		KERN_PANIC(KERN_ERROR_INVALID_TEXT_SIZE);
	kern_rodata_vaddr = (uint64_t)_kernel_rodata_start;
	kern_rodata_size = (uint64_t)(_kernel_rodata_end -
	    _kernel_rodata_start);
	if (!PAGE_ALIGNED(kern_rodata_size))
		KERN_PANIC(KERN_ERROR_INVALID_RODATA_SIZE);
	kern_data_vaddr = (uint64_t)_kernel_data_start;
	kern_data_size = (uint64_t)(_kernel_data_end - _kernel_data_start);
	if (!PAGE_ALIGNED(kern_data_size))
		KERN_PANIC(KERN_ERROR_INVALID_DATA_SIZE);
	kern_bss_vaddr = (uint64_t)_kernel_bss_start;
	kern_bss_size = (uint64_t)(_kernel_bss_end - _kernel_bss_start);
	if (!PAGE_ALIGNED(kern_bss_size))
		KERN_PANIC(KERN_ERROR_INVALID_BSS_SIZE);
	kern_arena_vaddr = (uint64_t)_kernel_arena_start;
	kern_arena_size = (uint64_t)(_kernel_arena_end - _kernel_arena_start);
	if (!PAGE_ALIGNED(kern_arena_size))
		KERN_PANIC(KERN_ERROR_INVALID_ARENA_SIZE);
	kern_stack_vaddr = (uint64_t)_kernel_stack_start;
	kern_stack_size = (uint64_t)(_kernel_stack_end - _kernel_stack_start);
	if (!PAGE_ALIGNED(kern_stack_size))
		KERN_PANIC(KERN_ERROR_INVALID_STACK_SIZE);
	return;
}

static void
kern_init_gdt(void)
{
	int ret = 0;
	struct gdt_entry_conf conf;
	SET_GDT_CONF_KERNEL_CODE(conf);
	ret = gdt_modify_entry(gdt, &conf, GDT_KERNEL_CODE_INDEX);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	SET_GDT_CONF_KERNEL_DATA(conf);
	ret = gdt_modify_entry(gdt, &conf, GDT_KERNEL_DATA_INDEX);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	SET_GDT_CONF_USER_CODE(conf);
	ret = gdt_modify_entry(gdt, &conf, GDT_USER_CODE_INDEX);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	SET_GDT_CONF_USER_DATA(conf);
	ret = gdt_modify_entry(gdt, &conf, GDT_USER_DATA_INDEX);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	SET_GDT_CONF_TSS(tss, conf);
	ret = gdt_modify_system_entry(gdt, &conf, GDT_TSS_INDEX);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	return;
}

static void
kern_init_tss(void)
{
	int ret = 0;
	/* TODO : tss */
	return;
}

static void
kern_init_idt(void)
{
	int ret = 0;
	/* TODO : idt */
	return;
}

static void
kern_set_tables(void)
{
	int ret = 0;
	gdt_set_table((uint64_t)gdt, (uint16_t)(GDT_SIZE - 1));
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	return;
}

static void
kern_verify_memtypes(const struct mem_info *mem)
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
		KERN_PANIC(KERN_ERROR_MISSING_AVAILABLE_MEMTYPE);
	if ((flags & 0x2) == 0)
		KERN_PANIC(KERN_ERROR_MISSING_BOOTLOADER_MEMTYPE);
	if ((flags & 0x4) == 0)
		KERN_PANIC(KERN_ERROR_MISSING_ACPI_RECLAIM_MEMTYPE);
	return;
}

static void
kern_init_pml4(void)
{
	int ret = 0;
	if (phys_pml4 == NULL) {
		ALLOC_PML4(phys_pml4, ret);
		if (RET_ERROR(ret))
			KERN_PANIC(ret);
	}
	return;
}

static void
kern_map_kernel(void *pdpt)
{
	if (phys_pml4 == NULL)
		KERN_PANIC(KERN_ERROR_INVALID_PML4);
	int pml4i = (int)((kern_vaddr >> 39) & 0x1ffULL);
	if (ENTRY_PRESENT(phys_pml4[pml4i]))
		KERN_PANIC(KERN_ERROR_PAGE_ALREADY_PRESENT);
	phys_pml4[pml4i] = ((uint64_t)pdpt & 0xffffffffffffULL) | PML4E_FLAGS;
	return;
}

static int
kern_map_desc(const struct mem_desc *desc)
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
kern_map_avail_desc(const struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_AVAILABLE) ? kern_map_desc(desc) : 0;
}

static int
kern_map_bootldr_desc(const struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_BOOTLOADER) ? kern_map_desc(desc) : 0;
}

static int
kern_map_acpi_desc(const struct mem_desc *desc)
{
	return ((desc->type == MEMTYPE_ACPI_RECLAIM) ||
	    (desc->type == MEMTYPE_ACPI_NVS)) ? kern_map_desc(desc) : 0;
}

static int
kern_iterate_memmap(const struct mem_info *mem,
    int (*action)(const struct mem_desc *))
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

static void
kern_map_physmem(const struct mem_info *mem)
{
	int ret = 0;
	ret = kern_iterate_memmap(mem, kern_map_avail_desc);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_iterate_memmap(mem, kern_map_bootldr_desc);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_iterate_memmap(mem, kern_map_acpi_desc);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	return;
}

static void
kern_secure_sections(void)
{
	int ret = 0;
	vmm_set_efer_nxe();
	ret = vmm_set_permission(kern_text_vaddr,
	    kern_text_vaddr + kern_text_size, 0x1, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = vmm_set_permission(kern_rodata_vaddr,
	    kern_rodata_vaddr + kern_rodata_size, 0x0, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = vmm_set_permission(kern_data_vaddr,
	    kern_data_vaddr + kern_data_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = vmm_set_permission(kern_bss_vaddr,
	    kern_bss_vaddr + kern_bss_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = vmm_set_permission(kern_arena_vaddr,
	    kern_arena_vaddr + kern_arena_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = vmm_set_permission(kern_stack_vaddr,
	    kern_stack_vaddr + kern_stack_size, 0x2, phys_pml4,
	    extract_physmem_vaddr);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	return;
}

static int
kern_disable_execperm(const struct mem_desc *desc, uint8_t perm)
{
	uint64_t vaddr_s = CONVERT_PHYSMEM_P2V(desc->phys_start);
	uint64_t vaddr_e = vaddr_s + desc->page_count * PAGE_SIZE;
	return vmm_set_permission(vaddr_s, vaddr_e, perm, phys_pml4,
	    extract_physmem_vaddr);
}

static int
kern_disable_avail_execperm(const struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_AVAILABLE) ?
	    kern_disable_execperm(desc, 0x2) : 0;
}

static int
kern_disable_bootldr_execperm(const struct mem_desc *desc)
{
	return (desc->type == MEMTYPE_BOOTLOADER) ?
	    kern_disable_execperm(desc, 0x2) : 0;
}

static int
kern_disable_acpi_execperm(const struct mem_desc *desc)
{
	return ((desc->type == MEMTYPE_ACPI_RECLAIM) ||
	    (desc->type == MEMTYPE_ACPI_NVS)) ?
	    kern_disable_execperm(desc, 0x0) : 0;
}

static void
kern_secure_physmem(const struct mem_info *mem)
{
	int ret = 0;
	ret = kern_iterate_memmap(mem, kern_disable_avail_execperm);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_iterate_memmap(mem, kern_disable_bootldr_execperm);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	ret = kern_iterate_memmap(mem, kern_disable_acpi_execperm);
	if (RET_ERROR(ret))
		KERN_PANIC(ret);
	return;
}

void
kern_main(struct kern_args *kargs)
{
	kern_init_addrs(kargs);
	kern_init_gdt();
	kern_init_tss();
	gdt_set_table((uint64_t)gdt, (uint16_t)GDT_SIZE);
	gdt_reload_table();
	tss_set_table((uint64_t)tss);
	tss_reload_table();
	kern_init_idt();
	kern_set_tables();
	kern_init_arena();
	kern_verify_memtypes(&kargs->mem);
	kern_init_pml4();
	kern_map_kernel(kargs->kern_pdpt);
	kern_map_physmem(&kargs->mem);
	vmm_set_pml4((uint64_t *)CONVERT_KERNEL_V2P(phys_pml4));
	kargs = (struct kern_args *)CONVERT_PHYSMEM_P2V(kargs);
	kargs->kern_pdpt = (void *)CONVERT_PHYSMEM_P2V(kargs->kern_pdpt);
	kargs->acpi = (void *)CONVERT_PHYSMEM_P2V(kargs->acpi);
	kargs->mem.info = (void *)CONVERT_PHYSMEM_P2V(kargs->mem.info);
	kern_secure_sections();
	kern_secure_physmem(&kargs->mem);
	return;
}
