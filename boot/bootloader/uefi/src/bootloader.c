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
				size_t count = 1, jump, tmp_num = num;
				while (tmp_num != 0) {
					tmp_num /= 10;
					count++;
				}
				jump = count - 1;
				while (--count) {
					formatted[j + count - 1] =
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
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MemoryMapSize = 0, MapKey, DescriptorSize;
	UINT32 DescriptorVersion;
	_stat = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
	    MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
	if (_stat != EFI_BUFFER_TOO_SMALL)
		FATAL_ERROR(L"GetMemoryMap() failed\r\n");
	_stat = SystemTable->BootServices->AllocatePool(EfiLoaderData,
	    DescriptorSize, (VOID **) &MemoryMap);
	if (EFI_ERROR(_stat))
		FATAL_ERROR(L"GetMemoryMap() failed at second time\r\n");
	efi_printf("hello%d%s\r\n", 0x1337, "foo");
	HALT();
	return _stat;
}
