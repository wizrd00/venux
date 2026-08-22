#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#undef NULL

#include <Uefi.h>

#define FORMATTED_SIZE 1024

#define CLEAR_SCREEN() SystemTable->ConOut->ClearScreen(SystemTable->ConOut)

#define HALT()\
	do {\
		SystemTable->ConOut->SetAttribute(SystemTable->ConOut,\
		    EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));\
		SystemTable->ConOut->OutputString(SystemTable->ConOut,\
		    L"Halting...\r\n");\
		SystemTable->ConOut->SetAttribute(SystemTable->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		while (1);\
	} while (0)

#define FATAL_ERROR(msg)\
	do {\
		SystemTable->ConOut->SetAttribute(SystemTable->ConOut,\
		    EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));\
		SystemTable->ConOut->OutputString(SystemTable->ConOut,\
		    L"[FATAL ERROR] -> " msg);\
		SystemTable->ConOut->SetAttribute(SystemTable->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		HALT();\
	} while (0)

#endif
