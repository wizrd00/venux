#ifndef _BOOTLOADER_GLOBAL_H
#define _BOOTLOADER_GLOBAL_H

#include "Uefi.h"

#define FORMATTED_SIZE 1024
#define BUFFER_SIZE 1024
#define PAGE_TABLES_COUNT 4
#define PAGE_TABLES_ENTRY_COUNT 512
#define KERNEL_NAME L"venux.elf"

extern EFI_HANDLE ImgHdl;
extern EFI_SYSTEM_TABLE *SysTab;
extern EFI_STATUS status;

#endif
