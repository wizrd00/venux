#include "bootloader.h"

EFI_HANDLE ImgHdl;
EFI_SYSTEM_TABLE *SysTab;
EFI_STATUS status = EFI_SUCCESS;
EFI_FILE_PROTOCOL *Volume = NULL;
EFI_FILE_PROTOCOL *KernelFile = NULL;
unsigned char *page_tables = NULL;
size_t kernel_size = 0;
size_t virt_kernel_base = 0;
size_t virt_kernel_start = 0;
size_t virt_kernel_end = 0;
size_t real_kernel_base = 0;
size_t real_kernel_start = 0;
size_t real_kernel_end = 0;

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
	
	status = Volume->Open(Volume, &KernelFile, KERNEL_NAME,
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
		return ret;

	struct boot_elf64_ehdr ehdr;
	size_t ehdr_size = sizeof(struct boot_elf64_ehdr);
	status = KernelFile->Read(KernelFile, (UINTN *)&ehdr_size,
	    (VOID *)&ehdr);
	if (EFI_ERROR(status)) {
		LOAD_KERNEL_CLEAN_UP();
		return ret = LOAD_ERROR_READ_FILE;
	}

	ret = efi_validate_elf(&ehdr);
	if (ret != 0) {
		LOAD_KERNEL_CLEAN_UP();
		return ret;
	}

	struct boot_elf64_phdr phdr;
	size_t phdr_size = sizeof(struct boot_elf64_phdr);
	status = KernelFile->SetPosition(KernelFile, ehdr.e_phoff);
	if (EFI_ERROR(status)) {
		LOAD_KERNEL_CLEAN_UP();
		return ret = LOAD_ERROR_SEEK_FILE;
	}

	bool valid_entry = false;
	UINT64 start = 0, end = 0;
	for (int i = 0; i < (int)ehdr.e_phnum; i++) {
		status = KernelFile->Read(KernelFile, (UINTN *)&phdr_size,
		    (VOID *)&phdr);
		if (EFI_ERROR(status)) {
			LOAD_KERNEL_CLEAN_UP();
			return ret = LOAD_ERROR_READ_FILE;
		}
		if (phdr.p_type != PT_LOAD)
			continue;
		if (phdr.p_memsz < phdr.p_filesz) {
			LOAD_KERNEL_CLEAN_UP();
			return ret = LOAD_ERROR_INVALID_PHDR;
		}
		if ((ehdr.e_entry >= phdr.p_vaddr) &&
		    (ehdr.e_entry < phdr.p_vaddr + phdr.p_memsz))
			valid_entry = true;
		if (i == 0) {
			start = phdr.p_vaddr;
			end = start + phdr.p_memsz;
		} else {
			start = (start < phdr.p_vaddr) ? start : phdr.p_vaddr;
			end  = (end > phdr.p_vaddr) ? end : phdr.p_vaddr;
		}
	}
	virt_kernel_start = (size_t)(start);
	virt_kernel_base = virt_kernel_start - STACK_SIZE;
	virt_kernel_end = (size_t)(end);
	kernel_size = virt_kernel_end - virt_kernel_start;

	if (kernel_size == 0) {
		LOAD_KERNEL_CLEAN_UP();
		return ret = LOAD_ERROR_NO_PTLOAD;
	}

	if (!valid_entry) {
		LOAD_KERNEL_CLEAN_UP();
		return ret = LOAD_ERROR_INVALID_ENTRY;
	}

	EFI_PHYSICAL_ADDRESS Memory;
	UINTN Pages = (UINTN)((kernel_size + STACK_SIZE) / 4096 + 1);
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, Pages, &Memory);
	if (EFI_ERROR(status)) {
		LOAD_KERNEL_CLEAN_UP();
		return ret = LOAD_ERROR_ALLOCATE_PAGE;
	}
	real_kernel_start = (size_t)Memory;
	real_kernel_base = real_kernel_start - STACK_SIZE;
	real_kernel_end = real_kernel_start + kernel_size;

	status = KernelFile->SetPosition(KernelFile, ehdr.e_phoff);
	if (EFI_ERROR(status)) {
		LOAD_KERNEL_CLEAN_UP();
		SysTab->BootServices->FreePages(Memory, Pages);
		return ret = LOAD_ERROR_SEEK_FILE;
	}

	for (int i = 0; i < (int)ehdr.e_phnum; i++) {
		status = KernelFile->Read(KernelFile, (UINTN *)&phdr_size,
		    (VOID *)&phdr);
		if (EFI_ERROR(status)) {
			LOAD_KERNEL_CLEAN_UP();
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_READ_FILE;
		}
		if (phdr.p_type != PT_LOAD)
			continue;

		if (((size_t)phdr.p_vaddr < virt_kernel_start) ||
		    ((size_t)phdr.p_vaddr >= virt_kernel_start + kernel_size)) {
			LOAD_KERNEL_CLEAN_UP();
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_INVALID_PHDR;
		}

		UINT64 pos;
		status = KernelFile->GetPosition(KernelFile, &pos);
		if (EFI_ERROR(status)) {
			LOAD_KERNEL_CLEAN_UP();
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_SEEK_FILE;
		}

		status = KernelFile->SetPosition(KernelFile, phdr.p_offset);
		if (EFI_ERROR(status)) {
			LOAD_KERNEL_CLEAN_UP();
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_SEEK_FILE;
		}

		size_t filesz = (size_t)phdr.p_filesz;
		size_t gapsz = (size_t)phdr.p_memsz - filesz;
		size_t addr = (size_t)Memory + STACK_SIZE +
		    ((size_t)phdr.p_vaddr - virt_kernel_start);
		while (filesz > 0) {
			UINTN readsz = (UINTN)((filesz < BUFFER_SIZE) ?
			    filesz : BUFFER_SIZE);
			status = KernelFile->Read(KernelFile, &readsz,
			    (VOID *)buffer);
			if (EFI_ERROR(status)) {
				LOAD_KERNEL_CLEAN_UP();
				SysTab->BootServices->FreePages(Memory, Pages);
				return ret = LOAD_ERROR_READ_FILE;
			}
			efi_memcpy((void *)addr, (void *)buffer, readsz);
			filesz -= readsz;
			addr += readsz;
		}
		efi_memset((void *)addr, 0, gapsz);

		status = KernelFile->SetPosition(KernelFile, pos);
		if (EFI_ERROR(status)) {
			LOAD_KERNEL_CLEAN_UP();
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_SEEK_FILE;
		}
	}
	LOAD_KERNEL_CLEAN_UP();
	return ret;
}

static int
efi_create_tables(void)
{
	int ret = 0;
	EFI_PHYSICAL_ADDRESS Memory;
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, (UINTN)PAGE_TABLES_COUNT, &Memory);
	if (EFI_ERROR(status))
		return ret = MAP_ERROR_ALLOCATE_PAGE;
	page_tables = (unsigned char *)Memory;
	efi_memset((void *)page_tables, 0, PAGE_TABLES_COUNT * 4096);
	return ret;
}

static int
efi_map_page(size_t real_addr, size_t virt_addr)
{
	int ret = 0;
	virt_addr = virt_addr & 0xffffffffffff;
	real_addr = real_addr & 0xffffffffffff;
	UINT64 *pml4 = (UINT64 *)page_tables;
	UINT64 *pdpt = pml4 + 512;
	UINT64 *pd = pdpt + 512;
	UINT64 *pt = pd + 512;
	int pml4e_index = (int)(virt_addr >> 39);
	int pdpte_index = (int)((virt_addr >> 30) & 0x1ff);
	int pde_index = (int)((virt_addr >> 21) & 0x1ff);
	int pte_index = (int)((virt_addr >> 12) & 0x1ff);
	pml4[pml4e_index] = (UINT64)(real_addr | 0x005);
	pdpt[pdpte_index] = pml4[pml4e_index];
	pd[pdpte_index] = pml4[pml4e_index];
	pt[pte_index] = pml4[pml4e_index];
	return ret;
}

static int
efi_map_kernel(void)
{
	int ret = 0;
	if (!PAGE_ALIGNED(real_kernel_base))
		ret = MAP_ERROR_PAGE_ALIGNED;
	if (!PAGE_ALIGNED(virt_kernel_base))
		ret = MAP_ERROR_PAGE_ALIGNED;
	ret = efi_create_tables();
	if (ret != 0)
		return ret;
	for (size_t real = real_kernel_base, virt = virt_kernel_base;
	    real < real_kernel_end; real += 4096, virt += 4096) {
		efi_map_page(real, virt);
	}
	/* TODO : implement efi_apply_tables() in assembly */
	return ret;
}

EFI_STATUS EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	int ret = 0;
	ImgHdl = ImageHandle;
	SysTab = SystemTable;
	CLEAR_SCREEN();

	ret = efi_load_kernel();
	if (ret != 0)
		LOAD_ERROR("efi_load_kernel() returned %d with EFI_STATUS %d",
		    ret, status);

	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MemoryMapSize = 0, MapKey, DescriptorSize;
	UINT32 DescriptorVersion;
	status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (status != EFI_BUFFER_TOO_SMALL)
		FATAL_ERROR("GetMemoryMap() must return EFI_BUFFER_TOO_SMALL"
		    "but returned status %d", status);
	/* TODO : allocate enough buffer to fit kernel args */
	MemoryMapSize += 2 * DescriptorSize;
	status = SystemTable->BootServices->AllocatePool(EfiLoaderData,
	    MemoryMapSize, (VOID **) &MemoryMap);
	if (EFI_ERROR(status))
		FATAL_ERROR("AllocatePool() failed with status %d", status);
	status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (EFI_ERROR(status))
		FATAL_ERROR("GetMemoryMap() failed with status %d", status);

	/* TODO : call ExitBootServices() */

	ret = efi_map_kernel();
	if (ret != 0)
		LOAD_ERROR("efi_map_kernel() returned %d with EFI_STATUS %d",
		    ret, status);
	HALT();
	return status;
}
