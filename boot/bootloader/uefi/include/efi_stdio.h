#ifndef _BOOTLOADER_EFI_STDIO_H
#define _BOOTLOADER_EFI_STDIO_H

#include <stdbool.h>
#include <stdarg.h>

#include "Uefi.h"

#include "globals.h"
#include "efi_string.h"

#define NUM_TO_STR(num)\
	if (num == 0) {\
		str[j++] = '0';\
		break;\
	}\
	if (num < 0) {\
		str[j++] = '-';\
		num = -num;\
	}\
	tmp_num = num;\
	while (tmp_num != 0) {\
		tmp_num /= 10;\
		count++;\
	}\
	tmp_num = num;\
	jump = count;\
	while (count > 0) {\
		str[j + count-- - 1] =\
		    (char)(tmp_num % 10) + '0';\
		tmp_num /= 10;\
	}\
	j += jump;

void efi_fputs(const char *restrict s, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut);

int efi_vsprintf(char *restrict str, const char *restrict format, va_list ap);

int efi_vprintf(const char *restrict format, va_list ap);

int efi_sprintf(char *restrict str, const char *restrict format, ...);

int efi_printf(const char *restrict format, ...);

#endif
