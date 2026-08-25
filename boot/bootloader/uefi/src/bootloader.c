#include "bootloader.h"

EFI_HANDLE ImgHdl;
EFI_SYSTEM_TABLE *SysTab;
EFI_STATUS status = EFI_SUCCESS;

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
efi_load_kernel_file(EFI_FILE_PROTOCOL **file)
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
efi_load_kernel(void)
{
	int ret = 0;
	EFI_FILE_PROTOCOL *KernelFile = NULL;
	ret = efi_load_kernel_file(&KernelFile);
	if (ret != 0)
		return ret;
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
