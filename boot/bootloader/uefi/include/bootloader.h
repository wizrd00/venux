#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#undef NULL

#include <Uefi.h>

#define FORMATTED_SIZE 1024

#define CLEAR_SCREEN() SysTab->ConOut->ClearScreen(SysTab->ConOut)

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
