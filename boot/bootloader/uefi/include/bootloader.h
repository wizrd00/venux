#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#undef NULL

#include <Uefi.h>

#define FORMATTED_SIZE 1024

#define CLEAR_SCREEN() SysTab->ConOut->ClearScreen(SysTab->ConOut)

#define NUM_TO_STR(num)\
	if (num == 0) {\
		formatted[j++] = '0';\
		break;\
	}\
	if (num < 0) {\
		formatted[j++] = '-';\
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
		formatted[j + count-- - 1] =\
		    (char)(tmp_num % 10) + '0';\
		tmp_num /= 10;\
	}\
	j += jump;

#define HALT()\
	do {\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));\
		SysTab->ConOut->OutputString(SysTab->ConOut,\
		    L"Halting...\r\n");\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		while (1);\
	} while (0)

#define FATAL_ERROR(msg)\
	do {\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));\
		SysTab->ConOut->OutputString(SysTab->ConOut,\
		    L"[FATAL ERROR] -> " msg);\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		HALT();\
	} while (0)

#endif
