#include "bootloader.h"

EFI_HANDLE ImgHdl;
EFI_SYSTEM_TABLE *SysTab;
EFI_STATUS status = EFI_SUCCESS;
EFI_FILE_PROTOCOL *Volume = NULL;
EFI_FILE_PROTOCOL *KernelFile = NULL;

UINT64 *pml4;

size_t efi_app_size = 0;
size_t efi_app_start = 0;
size_t efi_app_end = 0;

size_t kernel_size = 0;
size_t kernel_entry = 0;
size_t virt_kernel_start = 0;
size_t virt_kernel_end = 0;
size_t real_kernel_start = 0;
size_t real_kernel_end = 0;

struct kern_args kargs;

static int
efi_open_kernel_file(void)
{
	int ret = 0;
	EFI_LOADED_IMAGE *LoadedImage = NULL;
	EFI_GUID LoadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	status = SysTab->BootServices->HandleProtocol(ImgHdl, &LoadedImageGuid,
	    (VOID **) &LoadedImage);
	if (EFI_ERROR(status))
		return ret = LOAD_ERROR_HANDLE_PROTOCOL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
	EFI_GUID FileSystemGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	status = SysTab->BootServices->HandleProtocol(LoadedImage->DeviceHandle,
	    &FileSystemGuid, (VOID **) &FileSystem);
	if (EFI_ERROR(status))
		return ret = LOAD_ERROR_HANDLE_PROTOCOL;
	status = FileSystem->OpenVolume(FileSystem, &Volume);
	if (EFI_ERROR(status))
		return ret = LOAD_ERROR_OPEN_VOLUME;
	status = Volume->Open(Volume, &KernelFile, KERNEL_FILENAME,
	    EFI_FILE_MODE_READ, (UINT64)0);
	if (EFI_ERROR(status))
		return ret = LOAD_ERROR_OPEN_FILE;
	return ret;
}

static int
efi_validate_elf(const struct boot_elf64_ehdr *ehdr)
{
	int ret = 0;
	unsigned char exp_ident[] = {0x7f, 'E', 'L', 'F', ELFCLASS64,
	    ELFDATA2LSB, EV_CURRENT, ELFOSABI_SYSV, 0, 0, 0, 0, 0, 0, 0, 0};
	if (efi_memcmp((void *)ehdr->e_ident, (void *)exp_ident,
	    (size_t)ELFNIDENT) != 0)
		return ret = LOAD_ERROR_INVALID_ELF_IDENT;
	if (ehdr->e_type != (UINT16)ET_EXEC)
		return ret = LOAD_ERROR_INVALID_ELF_TYPE;
	if (ehdr->e_machine != (UINT16)EM_X86_64)
		return ret = LOAD_ERROR_INVALID_ELF_MACHINE;
	return ret;
}

static int
efi_load_kernel(void)
{
	int ret = 0;
	unsigned char buffer[BUFFER_SIZE];
	ret = efi_open_kernel_file();
	if (ret != 0)
		goto out;
	struct boot_elf64_ehdr ehdr;
	size_t ehdr_size = sizeof(struct boot_elf64_ehdr);
	status = KernelFile->Read(KernelFile, (UINTN *)&ehdr_size,
	    (VOID *)&ehdr);
	if (EFI_ERROR(status)) {
		ret = LOAD_ERROR_READ_FILE;
		goto out_close;
	}
	kernel_entry = (size_t)ehdr.e_entry;
	ret = efi_validate_elf(&ehdr);
	if (ret != 0)
		goto out_close;
	struct boot_elf64_phdr phdr;
	size_t phdr_size = sizeof(struct boot_elf64_phdr);
	status = KernelFile->SetPosition(KernelFile, ehdr.e_phoff);
	if (EFI_ERROR(status)) {
		ret = LOAD_ERROR_SEEK_FILE;
		goto out_close;
	}
	bool valid_entry = false;
	bool first_phdr = true;
	UINT64 start = 0, end = 0;
	for (int i = 0; i < (int)ehdr.e_phnum; i++) {
		status = KernelFile->Read(KernelFile, (UINTN *)&phdr_size,
		    (VOID *)&phdr);
		if (EFI_ERROR(status)) {
			ret = LOAD_ERROR_READ_FILE;
			goto out_close;
		}
		if (phdr.p_type != PT_LOAD)
			continue;
		if (phdr.p_vaddr == 0)
			continue;
		if (phdr.p_memsz < phdr.p_filesz) {
			ret = LOAD_ERROR_INVALID_PHDR;
			goto out_close;
		}
		if ((ehdr.e_entry >= phdr.p_vaddr) &&
		    (ehdr.e_entry < phdr.p_vaddr + phdr.p_memsz))
			valid_entry = true;
		if (first_phdr) {
			start = phdr.p_vaddr;
			end = start + phdr.p_memsz;
			first_phdr = false;
		} else {
			start = (start < phdr.p_vaddr) ? start : phdr.p_vaddr;
			end  = (end > phdr.p_vaddr + phdr.p_memsz) ?
			    end : phdr.p_vaddr + phdr.p_memsz;
		}
	}
	virt_kernel_start = (size_t)(start);
	virt_kernel_end = (size_t)(end);
	if (virt_kernel_start >= virt_kernel_end) {
		ret = LOAD_ERROR_INVALID_RANGE;
		goto out_close;
	}
	kernel_size = virt_kernel_end - virt_kernel_start;
	if (kernel_size == 0) {
		ret = LOAD_ERROR_NO_PTLOAD;
		goto out_close;
	}
	if (!valid_entry) {
		ret = LOAD_ERROR_INVALID_ENTRY;
		goto out_close;
	}
	EFI_PHYSICAL_ADDRESS Memory;
	UINTN Pages = (UINTN)((kernel_size / PAGE_SIZE) + 1);
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, Pages, &Memory);
	if (EFI_ERROR(status)) {
		ret = LOAD_ERROR_ALLOCATE_PAGE;
		goto out_close;
	}
	real_kernel_start = (size_t)Memory;
	real_kernel_end = real_kernel_start + kernel_size;
	status = KernelFile->SetPosition(KernelFile, ehdr.e_phoff);
	if (EFI_ERROR(status)) {
		ret = LOAD_ERROR_SEEK_FILE;
		goto out_free;
	}
	for (int i = 0; i < (int)ehdr.e_phnum; i++) {
		status = KernelFile->Read(KernelFile, (UINTN *)&phdr_size,
		    (VOID *)&phdr);
		if (EFI_ERROR(status)) {
			ret = LOAD_ERROR_READ_FILE;
			goto out_free;
		}
		if (phdr.p_type != PT_LOAD)
			continue;
		if (phdr.p_vaddr == 0)
			continue;
		if (((size_t)phdr.p_vaddr < virt_kernel_start) ||
		    ((size_t)phdr.p_vaddr >= virt_kernel_end)) {
			ret = LOAD_ERROR_INVALID_PHDR;
			goto out_free;
		}
		UINT64 pos;
		status = KernelFile->GetPosition(KernelFile, &pos);
		if (EFI_ERROR(status)) {
			ret = LOAD_ERROR_SEEK_FILE;
			goto out_free;
		}
		status = KernelFile->SetPosition(KernelFile, phdr.p_offset);
		if (EFI_ERROR(status)) {
			ret = LOAD_ERROR_SEEK_FILE;
			goto out_free;
		}
		size_t filesz = (size_t)phdr.p_filesz;
		size_t gapsz = (size_t)phdr.p_memsz - filesz;
		size_t addr = real_kernel_start +
		    ((size_t)phdr.p_vaddr - virt_kernel_start);
		while (filesz > 0) {
			UINTN readsz = (UINTN)((filesz < BUFFER_SIZE) ?
			    filesz : BUFFER_SIZE);
			status = KernelFile->Read(KernelFile, &readsz,
			    (VOID *)buffer);
			if (EFI_ERROR(status)) {
				ret = LOAD_ERROR_READ_FILE;
				goto out_free;
			}
			efi_memcpy((void *)addr, (void *)buffer, readsz);
			filesz -= readsz;
			addr += readsz;
		}
		efi_memset((void *)addr, 0, gapsz);
		status = KernelFile->SetPosition(KernelFile, pos);
		if (EFI_ERROR(status)) {
			ret = LOAD_ERROR_SEEK_FILE;
			goto out_free;
		}
	}
	out_normal :
		goto out_close;
	out_free :
		SysTab->BootServices->FreePages(Memory, Pages);
	out_close :
		KernelFile->Close(KernelFile);
		Volume->Close(Volume);
	out :
		return ret;
}

static int
efi_alloc_table(UINT64 *table)
{
	int ret = 0;
	EFI_PHYSICAL_ADDRESS Memory;
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, (UINT64)1, &Memory);
	if (EFI_ERROR(status))
		return ret = MAP_ERROR_ALLOCATE_PAGE;
	efi_memset((void *)Memory, 0, (size_t)PAGE_SIZE);
	*table = (UINT64)(Memory | ENTRY_FLAGS);
	return ret;
}

static int
efi_addto_pt(size_t real_start, size_t virt_start, size_t virt_end, UINT64 *pt)
{
	int ret = 0;
	while (virt_start < virt_end) {
		int pti = (int)((virt_start >> 12) & 0x1ffULL);
		if (ENTRY_PRESENT(pt[pti]))
			return ret = MAP_ERROR_PAGE_PRESENT;
		pt[pti] = (UINT64)(real_start | ENTRY_FLAGS);
		virt_start += 0x1000;
		real_start += 0x1000;
	}
	return ret;
}

static int
efi_addto_pd(size_t real_start, size_t virt_start, size_t virt_end, UINT64 *pd)
{
	int ret = 0;
	while (virt_start < virt_end) {
		size_t max_end = (virt_start | 0x1fffffULL) + 1;
		size_t chunk_end = (max_end > virt_end) ? virt_end : max_end;
		int pdi = (int)((virt_start >> 21) & 0x1ffULL);
		if (!ENTRY_PRESENT(pd[pdi])) {
			ret = efi_alloc_table(pd + pdi);
			if (ret != 0)
				return ret;
		}
		UINT64 *pt = (UINT64 *)(pd[pdi] & 0xfffffffffffff000ULL);
		ret = efi_addto_pt(real_start, virt_start, chunk_end, pt);
		if (ret != 0)
			return ret;
		real_start += chunk_end - virt_start;
		virt_start += chunk_end - virt_start;
	}
	return ret;
}

static int
efi_addto_pdpt(size_t real_start, size_t virt_start, size_t virt_end,
    UINT64 *pdpt)
{
	int ret = 0;
	int pdpti = (int)((virt_start >> 30) & 0x1ffULL);
	if (!ENTRY_PRESENT(pdpt[pdpti])) {
		ret = efi_alloc_table(pdpt + pdpti);
		if (ret != 0)
			return ret;
	}
	UINT64 *pd = (UINT64 *)(pdpt[pdpti] & 0xfffffffffffff000ULL);
	return efi_addto_pd(real_start, virt_start, virt_end, pd);
}

static int
efi_addto_pml4(size_t real_start, size_t virt_start, size_t virt_end)
{
	int ret = 0;
	int pml4i = (int)((virt_start >> 39) & 0x1ffULL);
	if (!ENTRY_PRESENT(pml4[pml4i])) {
		ret = efi_alloc_table(pml4 + pml4i);
		if (ret != 0)
			return ret;
	}
	UINT64 *pdpt = (UINT64 *)(pml4[pml4i] & 0xfffffffffffff000ULL);
	return efi_addto_pdpt(real_start, virt_start, virt_end, pdpt);
}

static int
efi_map_kernel(void)
{
	int ret = 0;
	if (!PAGE_ALIGNED(real_kernel_start))
		return ret = MAP_ERROR_PAGE_ALIGNED;
	if (!PAGE_ALIGNED(virt_kernel_start))
		return ret = MAP_ERROR_PAGE_ALIGNED;
	if (virt_kernel_start >= virt_kernel_end)
		return ret = MAP_ERROR_INVALID_RANGE;
	return efi_addto_pml4(real_kernel_start, virt_kernel_start,
	    virt_kernel_end);
}

static int
efi_map_efi_app(void)
{
	int ret = 0;
	if (!PAGE_ALIGNED(efi_app_start))
		return ret = MAP_ERROR_PAGE_ALIGNED;
	return efi_addto_pml4(efi_app_start, efi_app_start, efi_app_end);
}

static int
efi_map_mmap(void)
{
	int ret = 0;
	size_t mmap_start = (size_t)kargs.mem.info & 0xfffffffffffff000ULL;
	size_t mmap_end = (size_t)kargs.mem.info + kargs.mem.size *
	    kargs.mem.count;
	return efi_addto_pml4(mmap_start, mmap_start, mmap_end);
}

static int
efi_kargs_add_acpi(void)
{
	int ret = 0;
	EFI_GUID AcpiGuid = EFI_ACPI_TABLE_GUID;
	for (UINTN i = 0; i < SysTab->NumberOfTableEntries; i++) {
		if (efi_memcmp(
		    (void *)&SysTab->ConfigurationTable[i].VendorGuid,
		    (void *)&AcpiGuid, sizeof(EFI_GUID)) == 0) {
			kargs.acpi =
			    (void *)SysTab->ConfigurationTable[i].VendorTable;
			return ret;
		}
	}
	return ret = KARGS_ERROR_ACPI;
}

static int
efi_kargs_add_gop(void)
{
	int ret = 0;
	EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
	status = SysTab->BootServices->LocateProtocol(&GopGuid, NULL,
	    (VOID **)&Gop);
	if (EFI_ERROR(status))
		return ret = KARGS_ERROR_GOP;
	/* TODO */
	return ret;
}

static int
efi_kargs_add_kern_pdpt(void)
{
	int ret = 0;
	int pml4i = (int)((virt_kernel_start >> 39) & 0x1ffULL);
	kargs.kern_pdpt = (void *)(pml4[pml4i] & 0xfffffffffffff000ULL);
	return ret;
}

static inline int
convert_memtype(EFI_MEMORY_TYPE mem_type)
{
	switch (mem_type) {
	case EfiLoaderCode :
	case EfiBootServicesCode :
	case EfiBootServicesData :
	case EfiConventionalMemory :
	case EfiPersistentMemory :
		return AVAILABLE;
	case EfiReservedMemoryType :
	case EfiLoaderData :
	case EfiRuntimeServicesCode :
	case EfiRuntimeServicesData :
	case EfiUnusableMemory :
	case EfiMemoryMappedIO :
	case EfiMemoryMappedIOPortSpace :
	case EfiPalCode :
		return RESERVED;
	case EfiACPIReclaimMemory :
		return ACPI_RECLAIM;
	case EfiACPIMemoryNVS :
		return ACPI_NVS;
	default :
		return UNKNOWN;
	}
	return UNKNOWN;
}

static int
efi_kargs_add_mmap(UINTN *MapKey)
{
	int ret = 0;
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MemoryMapSize = 0, DescriptorSize;
	UINT32 DescriptorVersion;
	status = SysTab->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, MapKey, &DescriptorSize, &DescriptorVersion);
	if (status != EFI_BUFFER_TOO_SMALL)
		return ret = KARGS_ERROR_GET_MMAP0;
	MemoryMapSize += 2 * DescriptorSize;
	status = SysTab->BootServices->AllocatePool(EfiLoaderData,
	    MemoryMapSize, (VOID **) &MemoryMap);
	if (EFI_ERROR(status))
		return ret = KARGS_ERROR_ALLOCATE_POOL;
	kargs.mem.info = (void *)MemoryMap;
	kargs.mem.size = (int)DescriptorSize;
	kargs.mem.count = (int)(MemoryMapSize / DescriptorSize);
	ret = efi_map_mmap();
	if (ret != 0)
		return ret;
	status = SysTab->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, MapKey, &DescriptorSize, &DescriptorVersion);
	if (EFI_ERROR(status))
		return ret = KARGS_ERROR_GET_MMAP1;
	kargs.mem.size = (int)DescriptorSize;
	kargs.mem.count = (int)(MemoryMapSize / DescriptorSize);
	for (UINT8 *ptr = (UINT8 *)MemoryMap;
	    (UINTN)ptr < (UINTN)MemoryMap + MemoryMapSize;
	    ptr += DescriptorSize) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)ptr;
		desc->Type = convert_memtype(desc->Type);
	}
	return ret;
}

EFI_STATUS
efi_init_pml4(void)
{
	EFI_PHYSICAL_ADDRESS Memory;
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, (UINT64)1, &Memory);
	if (EFI_ERROR(status))
		return status;
	pml4 = (UINT64 *)Memory;
	efi_memset((void *)pml4, 0, (size_t)PAGE_SIZE);
	return status;
}

EFI_STATUS EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	int ret = 0;
	ImgHdl = ImageHandle;
	SysTab = SystemTable;
	EFI_LOADED_IMAGE *LoadedImage = NULL;
	EFI_GUID LoadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	CLEAR_SCREEN();
	status = SysTab->BootServices->HandleProtocol(ImgHdl, &LoadedImageGuid,
	    (VOID **) &LoadedImage);
	if (EFI_ERROR(status))
		FATAL_ERROR("HandleProtocol() failed to gather information"
		    " about the image with status %d", status);
	efi_app_size = (size_t)LoadedImage->ImageSize;
	efi_app_start = (size_t)LoadedImage->ImageBase;
	efi_app_end = efi_app_start + efi_app_size;
	status = efi_init_pml4();
	if (EFI_ERROR(status))
		FATAL_ERROR("efi_init_pml4() failed with EFI_STATUS %d",
		    status);
	ret = efi_load_kernel();
	if (ret != 0)
		LOAD_ERROR("efi_load_kernel() returned %d with EFI_STATUS %d",
		    ret, status);
	kargs.kern_start = (void *)real_kernel_start;
	if (virt_kernel_start <= efi_app_end)
		FATAL_ERROR("kernel start virtual address starts before"
		    " EFI application virtual address ends");
	ret = efi_map_kernel();
	if (ret != 0)
		MAP_ERROR("efi_map_kernel() returned %d with EFI_STATUS %d",
		    ret, status);
	efi_kargs_add_kern_pdpt();
	ret = efi_map_efi_app();
	if (ret != 0)
		MAP_ERROR("efi_map_efi_app() returned %d with EFI_STATUS %d",
		    ret, status);
	UINTN MapKey;
	ret = efi_kargs_add_mmap(&MapKey);
	if (ret != 0)
		FATAL_ERROR("efi_kargs_add_mmap() returned %d"
		    " with EFI_STATUS %d", ret, status);
	int trycount = 3;
	while (trycount-- > 0) {
		status = SysTab->BootServices->ExitBootServices(ImgHdl, MapKey);
		if (!EFI_ERROR(status))
			break;
	}
	if (trycount == 0)
		FATAL_ERROR("ExitBootServices() failed with EFI_STATUS %d",
		    status);
	MODIFY_SYSTAB();
	efi_leave();
	efi_halt();
	return status;
}
