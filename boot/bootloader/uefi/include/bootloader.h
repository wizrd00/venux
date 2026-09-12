#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include "Uefi.h"
#include "Protocol/SimpleFileSystem.h"
#include "Protocol/LoadedImage.h"

#include "globals.h"
#include "efi_stdio.h"
#include "efi_string.h"
#include "leave.h"
#include "halt.h"
#include "elf.h"

#define PML4_ENTRY_FLAGS 0x003
#define PDPT_ENTRY_FLAGS 0x003
#define PD_ENTRY_FLAGS 0x003
#define PT_ENTRY_FLAGS 0x003

#define LOAD_ERROR_HANDLE_PROTOCOL 1
#define LOAD_ERROR_OPEN_VOLUME 2
#define LOAD_ERROR_OPEN_FILE 3
#define LOAD_ERROR_READ_FILE 4
#define LOAD_ERROR_SEEK_FILE 5
#define LOAD_ERROR_ALLOCATE_PAGE 6
#define LOAD_ERROR_INVALID_ELF_IDENT 7
#define LOAD_ERROR_INVALID_ELF_TYPE 8
#define LOAD_ERROR_INVALID_ELF_MACHINE 9
#define LOAD_ERROR_INVALID_ENTRY 10
#define LOAD_ERROR_INVALID_PHDR 11
#define LOAD_ERROR_NO_PTLOAD 12

#define MAP_ERROR_PAGE_ALIGNED 1
#define MAP_ERROR_ALLOCATE_PAGE 2
#define MAP_ERROR_ALLOCATE_POOL 3

#define PAGE_ALIGNED(addr) ((addr & 0xfff) == 0)

#define CLEAR_SCREEN() SysTab->ConOut->ClearScreen(SysTab->ConOut)

#define MODIFY_SYSTAB()\
	do {\
		SysTab->ConsoleInHandle = NULL;\
		SysTab->ConIn = NULL;\
		SysTab->ConsoleOutHandle = NULL;\
		SysTab->ConOut = NULL;\
		SysTab->StandardErrorHandle = NULL;\
		SysTab->StdErr = NULL;\
		SysTab->BootServices = NULL;\
	} while (0);

#define HALT()\
	do {\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_RED, EFI_BLACK));\
		efi_printf("Halting...\r\n");\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		efi_halt();\
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

#define MAP_ERROR(...)\
	do {\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_BLUE, EFI_BLACK));\
		efi_printf("[MAP ERROR] -> " __VA_ARGS__);\
		efi_printf("\r\n");\
		SysTab->ConOut->SetAttribute(SysTab->ConOut,\
		    EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));\
		HALT();\
	} while (0)

#endif
