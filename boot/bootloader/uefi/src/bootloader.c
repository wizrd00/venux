#include "bootloader.h"

EFI_HANDLE ImgHdl;
EFI_SYSTEM_TABLE *SysTab;
EFI_STATUS status = EFI_SUCCESS;
size_t kernel_size = 0;
size_t kernel_start = 0;
size_t kernel_end = 0;

static void *
efi_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *tmp_dst = (unsigned char *)dst;
	const unsigned char *tmp_src = (const unsigned char *)src;
	while (n > 0) {
		*tmp_dst++ = *tmp_src++;
		n--;
	}
	return dst;
}

static void *
efi_memset(void *s, int c, size_t n)
{
	unsigned char *tmp_s = (unsigned char *)s;
	while (n > 0) {
		*tmp_s++ = (unsigned char)c;
		n--;
	}
	return s;
}

static int
efi_memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *tmp_s1 = (const unsigned char *)s1;
	const unsigned char *tmp_s2 = (const unsigned char *)s2;
	while (n > 0) {
		if ((*tmp_s1 - *tmp_s2) != 0)
			return (int)(*tmp_s1 - *tmp_s2);
		tmp_s1++;
		tmp_s2++;
		n--;
	}
	return 0;
}

static size_t
efi_strlen(const char *s)
{
	size_t len = 0;
	while (*s++ != '\0')
		len++;
	return len;
}

static void
efi_fputs(const char *restrict s, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut)
{
	CHAR16 wide_formatted[FORMATTED_SIZE];
	CHAR16 *ptr = wide_formatted;
	while (*s != '\0') {
		*ptr = (CHAR16)*s;
		s++;
		ptr++;
	}
	*ptr = (CHAR16)'\0';
	ConOut->OutputString(ConOut, wide_formatted);
	return;
}

static int
efi_vsprintf(char *restrict str, const char *restrict format, va_list ap)
{
	size_t fmt_size = efi_strlen(format) + 1;
	bool special = false;
	int count = 0, jump;
	size_t tmp_num, j = 0;
	for (size_t i = 0; i < fmt_size; i++) {
		if (special) {
			special = false;
			switch (format[i]) {
			case 's' :
				char *str_char = va_arg(ap, char *);
				while(*str_char != '\0')
					str[j++] = *str_char++;
				break;
			case 'd' :
				int num_int = va_arg(ap, int);
				NUM_TO_STR(num_int);
				break;
			case 'u' :
				unsigned int num_uint = va_arg(ap, unsigned int);
				NUM_TO_STR(num_uint);
				break;
			case 'l' :
				unsigned long num_ulong = va_arg(ap,
				    unsigned long);
				NUM_TO_STR(num_ulong);
				break;
			case 'z' :
				size_t num_size = va_arg(ap, size_t);
				NUM_TO_STR(num_size);
				break;
			default :
				str[j++] = '%';
				str[j++] = format[i];
				break;
			}
			continue;
		}
		if (format[i] == '%')
			special = true;
		else
			str[j++] = format[i];
	}
	str[j - 1] = '\0';
	return (int)j;
}

static int
efi_vprintf(const char *restrict format, va_list ap)
{
	char formatted[FORMATTED_SIZE];
	int result = efi_vsprintf(formatted, format, ap);
	efi_fputs(formatted, SysTab->ConOut);
	return result;
}

static int
efi_sprintf(char *restrict str, const char *restrict format, ...)
{
	va_list ap;
	int result;
	va_start(ap, format);
	result = efi_vsprintf(str, format, ap);
	va_end(ap);
	return result;
}

static int
efi_printf(const char *restrict format, ...)
{
	va_list ap;
	int result;
	va_start(ap, format);
	result = efi_vprintf(format, ap);
	va_end(ap);
	return result;
}

static int
efi_open_kernel_file(EFI_FILE_PROTOCOL **file)
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

	EFI_FILE_PROTOCOL *Volume = NULL;
	status = FileSystem->OpenVolume(FileSystem, &Volume);
	if (EFI_ERROR(status))
		return ret = LOAD_ERROR_OPEN_VOLUME;
	
	EFI_FILE_PROTOCOL *KernelFile = NULL;
	status = Volume->Open(Volume, &KernelFile, KERNEL_NAME,
	    EFI_FILE_MODE_READ, (UINT64)0);
	if (EFI_ERROR(status))
		return ret = LOAD_ERROR_OPEN_FILE;
	*file = KernelFile;
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
	EFI_FILE_PROTOCOL *KernelFile = NULL;
	ret = efi_open_kernel_file(&KernelFile);
	if (ret != 0)
		return ret;

	struct boot_elf64_ehdr ehdr;
	size_t ehdr_size = sizeof(struct boot_elf64_ehdr);
	status = KernelFile->Read(KernelFile, (UINTN *)&ehdr_size,
	    (VOID *)&ehdr);
	if (EFI_ERROR(status)) {
		KernelFile->Close(KernelFile);
		return ret = LOAD_ERROR_READ_FILE;
	}

	ret = efi_validate_elf(&ehdr);
	if (ret != 0) {
		KernelFile->Close(KernelFile);
		return ret;
	}

	struct boot_elf64_phdr phdr;
	size_t phdr_size = sizeof(struct boot_elf64_phdr);
	status = KernelFile->SetPosition(KernelFile, ehdr.e_phoff);
	if (EFI_ERROR(status)) {
		KernelFile->Close(KernelFile);
		return ret = LOAD_ERROR_SEEK_FILE;
	}

	bool valid_entry = false;
	UINT64 start = 0, end = 0;
	for (int i = 0; i < (int)ehdr.e_phnum; i++) {
		status = KernelFile->Read(KernelFile, (UINTN *)&phdr_size,
		    (VOID *)&phdr);
		if (EFI_ERROR(status)) {
			KernelFile->Close(KernelFile);
			return ret = LOAD_ERROR_READ_FILE;
		}
		if (phdr.p_type != PT_LOAD)
			continue;
		if (phdr.p_memsz < phdr.p_filesz) {
			KernelFile->Close(KernelFile);
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
	kernel_start = (size_t)(start);
	kernel_end = (size_t)(end);
	kernel_size = kernel_end - kernel_start;

	if (kernel_size == 0) {
		KernelFile->Close(KernelFile);
		return ret = LOAD_ERROR_NO_PTLOAD;
	}

	if (!valid_entry) {
		KernelFile->Close(KernelFile);
		return ret = LOAD_ERROR_INVALID_ENTRY;
	}

	EFI_PHYSICAL_ADDRESS Memory;
	UINTN Pages = (UINTN)((kernel_size + STACK_SIZE) / 4096 + 1);
	status = SysTab->BootServices->AllocatePages(AllocateAnyPages,
	    EfiLoaderData, Pages, &Memory);
	if (EFI_ERROR(status)) {
		KernelFile->Close(KernelFile);
		return ret = LOAD_ERROR_ALLOCATE_PAGE;
	}

	status = KernelFile->SetPosition(KernelFile, ehdr.e_phoff);
	if (EFI_ERROR(status)) {
		KernelFile->Close(KernelFile);
		SysTab->BootServices->FreePages(Memory, Pages);
		return ret = LOAD_ERROR_SEEK_FILE;
	}

	for (int i = 0; i < (int)ehdr.e_phnum; i++) {
		status = KernelFile->Read(KernelFile, (UINTN *)&phdr_size,
		    (VOID *)&phdr);
		if (EFI_ERROR(status)) {
			KernelFile->Close(KernelFile);
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_READ_FILE;
		}
		if (phdr.p_type != PT_LOAD)
			continue;

		if (((size_t)phdr.p_vaddr < kernel_start) ||
		    ((size_t)phdr.p_vaddr >= kernel_start + kernel_size)) {
			KernelFile->Close(KernelFile);
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_INVALID_PHDR;
		}

		UINT64 pos;
		status = KernelFile->GetPosition(KernelFile, &pos);
		if (EFI_ERROR(status)) {
			KernelFile->Close(KernelFile);
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_SEEK_FILE;
		}

		status = KernelFile->SetPosition(KernelFile, phdr.p_offset);
		if (EFI_ERROR(status)) {
			KernelFile->Close(KernelFile);
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_SEEK_FILE;
		}

		size_t filesz = (size_t)phdr.p_filesz;
		size_t gapsz = (size_t)phdr.p_memsz - filesz;
		size_t addr = (size_t)Memory + STACK_SIZE +
		    ((size_t)phdr.p_vaddr - kernel_start);
		while (filesz > 0) {
			UINTN readsz = (UINTN)((filesz < BUFFER_SIZE) ?
			    filesz : BUFFER_SIZE);
			status = KernelFile->Read(KernelFile, &readsz,
			    (VOID *)buffer);
			if (EFI_ERROR(status)) {
				KernelFile->Close(KernelFile);
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
			KernelFile->Close(KernelFile);
			SysTab->BootServices->FreePages(Memory, Pages);
			return ret = LOAD_ERROR_SEEK_FILE;
		}
	}
	return ret;
}

EFI_STATUS EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	int ret = 0;
	ImgHdl = ImageHandle;
	SysTab = SystemTable;
	CLEAR_SCREEN();
	/*
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MemoryMapSize = 0, MapKey, DescriptorSize;
	UINT32 DescriptorVersion;
	status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	MemoryMapSize += 2 * DescriptorSize;
	status = SystemTable->BootServices->AllocatePool(EfiLoaderData,
	    MemoryMapSize, (VOID **) &MemoryMap);
	status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	*/
	ret = efi_load_kernel();
	if (ret != 0)
		LOAD_ERROR("efi_load_kernel() returned %d with EFI_STATUS %d",
		    ret, status);
	HALT();
	return status;
}
