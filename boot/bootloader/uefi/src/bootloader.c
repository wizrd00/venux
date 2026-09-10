#include "bootloader.h"

EFI_HANDLE ImgHdl;
EFI_SYSTEM_TABLE *SysTab;
EFI_STATUS status = EFI_SUCCESS;
EFI_FILE_PROTOCOL *Volume = NULL;
EFI_FILE_PROTOCOL *KernelFile = NULL;

UINT64 *pml4;
UINT64 *pdpt0, *pdpt1;
UINT64 *pd0, *pd1;
UINT64 *pt0, *pt1;

size_t efi_app_size = 0;
size_t efi_app_start = 0;
size_t efi_app_end = 0;

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
		goto out;
	struct boot_elf64_ehdr ehdr;
	size_t ehdr_size = sizeof(struct boot_elf64_ehdr);
	status = KernelFile->Read(KernelFile, (UINTN *)&ehdr_size,
	    (VOID *)&ehdr);
	if (EFI_ERROR(status)) {
		ret = LOAD_ERROR_READ_FILE;
		goto out_close;
	}
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
		if (phdr.p_memsz < phdr.p_filesz) {
			ret = LOAD_ERROR_INVALID_PHDR;
			goto out_close;
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
		return ret = LOAD_ERROR_NO_PTLOAD;
		goto out_close;
	}
	if (!valid_entry) {
		ret = LOAD_ERROR_INVALID_ENTRY;
		goto out_close;
	}
	EFI_PHYSICAL_ADDRESS Memory;
	UINTN Pages = (UINTN)((kernel_size + STACK_SIZE) / 4096 + 1);
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, Pages, &Memory);
	if (EFI_ERROR(status)) {
		ret = LOAD_ERROR_ALLOCATE_PAGE;
		goto out_close;
	}
	real_kernel_base = (size_t)Memory;
	real_kernel_start = real_kernel_base + STACK_SIZE;
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
		if (((size_t)phdr.p_vaddr < virt_kernel_start) ||
		    ((size_t)phdr.p_vaddr >= virt_kernel_start + kernel_size)) {
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
	out_free :
		SysTab->BootServices->FreePages(Memory, Pages);
	out_close :
		KernelFile->Close(KernelFile);
		Volume->Close(Volume);
	out :
		return ret;
}

static int
efi_map_page(size_t real_addr, size_t virt_addr, UINT64 *pt)
{
	int ret = 0;
	real_addr = real_addr & 0xffffffffffff;
	virt_addr = virt_addr & 0xffffffffffff;
	unsigned int pt_index = (unsigned int)((virt_addr >> 12) & 0x1ff);
	pt[pt_index] = (UINT64)real_addr | PT_ENTRY_FLAGS;
	return ret;
}

static int
efi_map_kernel(void)
{
	int ret = 0;
	if (!PAGE_ALIGNED(real_kernel_base))
		return ret = MAP_ERROR_PAGE_ALIGNED;
	if (!PAGE_ALIGNED(virt_kernel_base))
		return ret = MAP_ERROR_PAGE_ALIGNED;
	EFI_PHYSICAL_ADDRESS Memory;
	UINT64 Pages = (UINT64)(PAGE_TABLES_COUNT - 1);
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, Pages, &Memory);
	if (EFI_ERROR(status))
		return ret = MAP_ERROR_ALLOCATE_PAGE;
	efi_memset((void *)Memory, 0, (size_t)Pages * 4096);
	pdpt0 = (UINT64 *)Memory;
	pd0 = pdpt0 + 512;
	pt0 = pd0 + 512;
	int pml4_index = (int)(virt_kernel_start >> 39);
	int pdpt_index = (int)((virt_kernel_start >> 30) & 0x1ff);
	int pd_index = (int)((virt_kernel_start >> 21) & 0x1ff);
	pml4[pml4_index] = (UINT64)pdpt0 | PML4_ENTRY_FLAGS;
	pdpt0[pdpt_index] = (UINT64)pd0 | PDPT_ENTRY_FLAGS;
	pd0[pd_index] = (UINT64)pt0 | PD_ENTRY_FLAGS;
	for (size_t real = real_kernel_start, virt = virt_kernel_start;
	    real < real_kernel_end; real += 4096, virt += 4096)
		efi_map_page(real, virt, pt0);
	return ret;
}

static int
efi_map_efi_app(void)
{
	int ret = 0;
	if (!PAGE_ALIGNED(efi_app_start))
		return ret = MAP_ERROR_PAGE_ALIGNED;
	EFI_PHYSICAL_ADDRESS Memory;
	UINT64 Pages = (UINT64)(PAGE_TABLES_COUNT - 1);
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, Pages, &Memory);
	if (EFI_ERROR(status))
		return ret = MAP_ERROR_ALLOCATE_PAGE;
	efi_memset((void *)Memory, 0, (size_t)Pages * 4096);
	pdpt1 = (UINT64 *)Memory;
	pd1 = pdpt1 + 512;
	pt1 = pd1 + 512;
	int pml4_index = (int)(efi_app_start >> 39);
	int pdpt_index = (int)((efi_app_start >> 30) & 0x1ff);
	int pd_index = (int)((efi_app_start >> 21) & 0x1ff);
	pml4[pml4_index] = (UINT64)pdpt1 | PML4_ENTRY_FLAGS;
	pdpt1[pdpt_index] = (UINT64)pd1 | PDPT_ENTRY_FLAGS;
	pd1[pd_index] = (UINT64)pt1 | PD_ENTRY_FLAGS;
	for (size_t i = efi_app_start; i < efi_app_end; i += 4096)
		efi_map_page(i, i, pt1);
	return ret;
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
	EFI_PHYSICAL_ADDRESS Memory;
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, (UINT64)1, &Memory);
	if (EFI_ERROR(status))
		FATAL_ERROR("AllocatePages() failed to allocate one page for"
		"pml4 with status %d", status);
	pml4 = (UINT64 *)Memory;
	efi_memset((void *)pml4, 0, (size_t)PAGE_SIZE);

	/* TODO validate efi_app_start to not to be in (kern_start, kern_end) */

	ret = efi_load_kernel();
	if (ret != 0)
		LOAD_ERROR("efi_load_kernel() returned %d with EFI_STATUS %d",
		    ret, status);
	ret = efi_map_kernel();
	if (ret != 0)
		MAP_ERROR("efi_map_kernel() returned %d with EFI_STATUS %d",
		    ret, status);
	ret = efi_map_efi_app();
	if (ret != 0)
		MAP_ERROR("efi_map_efi_app() returned %d with EFI_STATUS %d",
		    ret, status);
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MemoryMapSize = 0, MapKey, DescriptorSize;
	UINT32 DescriptorVersion;
	status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (status != EFI_BUFFER_TOO_SMALL)
		FATAL_ERROR("GetMemoryMap() must return EFI_BUFFER_TOO_SMALL"
		    "but returned status %d", status);
	/* TODO allocate enough buffer to fit kernel args */
	MemoryMapSize += 2 * DescriptorSize;
	status = SystemTable->BootServices->AllocatePool(EfiLoaderData,
	    MemoryMapSize, (VOID **) &MemoryMap);
	if (EFI_ERROR(status))
		FATAL_ERROR("AllocatePool() failed with status %d", status);
	status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (EFI_ERROR(status))
		FATAL_ERROR("GetMemoryMap() failed with status %d", status);
	status = SysTab->BootServices->ExitBootServices(ImgHdl, MapKey);
	if (EFI_ERROR(status))
		FATAL_ERROR("ExitBootServices() failed with status %d", status);
	MODIFY_SYSTAB();
	efi_exit();
	efi_halt();
	return status;
}
