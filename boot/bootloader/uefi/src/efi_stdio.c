#include "efi_stdio.h"

void
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

int
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

int
efi_vprintf(const char *restrict format, va_list ap)
{
	char formatted[FORMATTED_SIZE];
	int result = efi_vsprintf(formatted, format, ap);
	efi_fputs(formatted, SysTab->ConOut);
	return result;
}

int
efi_sprintf(char *restrict str, const char *restrict format, ...)
{
	va_list ap;
	int result;
	va_start(ap, format);
	result = efi_vsprintf(str, format, ap);
	va_end(ap);
	return result;
}

int
efi_printf(const char *restrict format, ...)
{
	va_list ap;
	int result;
	va_start(ap, format);
	result = efi_vprintf(format, ap);
	va_end(ap);
	return result;
}
