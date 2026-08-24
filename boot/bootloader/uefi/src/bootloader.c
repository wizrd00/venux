#include "bootloader.h"

EFI_HANDLE ImgHdl;
EFI_SYSTEM_TABLE *SysTab;

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
efi_vprintf(const char *restrict format, va_list ap)
{
	char formatted[FORMATTED_SIZE];
	size_t fmt_size = efi_strlen(format) + 1;
	bool special = false;
	char *str;
	size_t num, j = 0;
	for (size_t i = 0; i < fmt_size; i++) {
		if (special) {
			special = false;
			switch (format[i]) {
			case 's' :
				str = va_arg(ap, char *);
				while(*str != '\0')
					formatted[j++] = *str++;
				break;
			case 'd' :
				num = va_arg(ap, size_t);
				int count = 0, jump;
				if (num == 0) {
					formatted[j++] = '0';
					break;
				}
				size_t tmp_num = num;
				while (tmp_num != 0) {
					tmp_num /= 10;
					count++;
				}
				jump = count;
				while (count > 0) {
					formatted[j + count-- - 1] =
					    (char)(num % 10) + '0';
					num /= 10;
				}
				j += jump;
				break;
			default :
				formatted[j++] = '%';
				formatted[j++] = format[i];
				break;
			}
			continue;
		}
		if (format[i] == '%')
			special = true;
		else
			formatted[j++] = format[i];
	}
	formatted[j - 1] = '\0';
	efi_fputs(formatted, SysTab->ConOut);
	return (int)j;
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

EFI_STATUS EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS _stat = EFI_SUCCESS;
	ImgHdl = ImageHandle;
	SysTab = SystemTable;
	CLEAR_SCREEN();
	size_t TotalMemorySize = 0;
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MemoryMapSize = 0, MapKey, DescriptorSize;
	UINT32 DescriptorVersion;
	_stat = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (_stat != EFI_BUFFER_TOO_SMALL)
		FATAL_ERROR(L"GetMemoryMap() must return "
		    "EFI_BUFFER_TOO_SMALL but it didn't\r\n");

	MemoryMapSize += 2 * DescriptorSize;
	_stat = SystemTable->BootServices->AllocatePool(EfiLoaderData,
	    MemoryMapSize, (VOID **) &MemoryMap);
	if (EFI_ERROR(_stat))
		FATAL_ERROR(L"AllocatePool() failed\r\n");

	_stat = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (EFI_ERROR(_stat))
		FATAL_ERROR(L"GetMemoryMap() failed\r\n");

	/*
	efi_printf("start...\r\n");
	efi_printf("[%d:%dK] %d->%d\r\n", 0, 1234567, 12345612345, 765432);
	efi_printf("[%d:%dK] %d->%d\r\n", 18, 0, 39876543234, 765432);
	efi_printf("[%d:%dK] %d->%d\r\n", 20, 7654321, 0, 765432);
	efi_printf("[%d:%dK] %d->%d\r\n", 48, 9876544, 20593867987, 0);
	*/
	for (UINTN i = 0; i < DescriptorSize; i++) {
		EFI_MEMORY_DESCRIPTOR *md = MemoryMap + i;
		efi_printf("[%d:%dK] %d->%d\r\n", (size_t)i,
		    md->NumberOfPages * 4, (size_t)md->PhysicalStart,
		    (size_t)md->PhysicalStart + md->NumberOfPages * 4096);
		TotalMemorySize += (size_t)md->NumberOfPages * 4096;
	}
	efi_printf("TotalMemorySize = %d\r\n", TotalMemorySize);
	HALT();
	return _stat;
}
