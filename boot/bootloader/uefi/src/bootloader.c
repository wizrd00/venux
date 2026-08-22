#include "bootloader.h"

static size_t
efi_strlen(const char *s)
{
	size_t len = 0;
	while (*s++ != '\0')
		len++;
	return len;
}

static int
efi_vprintf(const char *restrict format, va_list ap)
{
	char formatted[FORMATTED_SIZE];
	size_t fmt_size = efi_strlen(format) + 1;
	bool skip = false, special = false;
	char *str;
	size_t num, digit, j = 0;
	for (size_t i = 0; i < fmt_size; i++) {
		if (skip) {
			skip = false;
			formatted[j] = format[i];
			j++;
			continue;
		}
		if (special) {
			special = false;
			switch (format[i]) {
			case 's' :
				str = va_arg(ap, char *);
				while(*str != '\0')
					formatted[j++] = *str++;
			case 'z' :
				num = va_arg(ap, size_t);
				size_t count = 1, jump = 0, tmp_num = num;
				while (tmp_num != 0) {
					tmp_num /= 10;
					count++;
					jump++;
				}
				while (count--) {
					digit = num % 10;
					num /= 10;
					formatted[j + count] = (char) digit;
				}
				j += jump;
			default :
				return -1;
			}
		}
		if (format[i] == '\\') {
			skip = true;
		} else if (format[i] == '%') {
			special = true;
		}
	}
	formatted[j] = '\0';
	return 0;
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
	efi_printf("hello%s", "hello");
	return _stat;
}
