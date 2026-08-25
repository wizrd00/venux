#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

#include "elf.h"

#define LOAD_ERROR_HANDLE_PROTOCOL 1
#define LOAD_ERROR_OPEN_VOLUME 2
#define LOAD_ERROR_OPEN_FILE 3

#define FORMATTED_SIZE 1024
#define KERNEL_NAME L"venux.elf"

#define CLEAR_SCREEN() SysTab->ConOut->ClearScreen(SysTab->ConOut)

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

#define HALT()\
	do {\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));\
		efi_printf("Halting...\r\n");\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		while (1);\
	} while (0)

#define FATAL_ERROR(...)\
	do {\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));\
		efi_printf("[FATAL ERROR] -> " __VA_ARGS__);\
		efi_printf("\r\n");\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		HALT();\
	} while (0)

#define LOAD_ERROR(...)\
	do {\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));\
		efi_printf("[LOAD ERROR] -> " __VA_ARGS__);\
		efi_printf("\r\n");\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		HALT();\
	} while (0)

#endif
