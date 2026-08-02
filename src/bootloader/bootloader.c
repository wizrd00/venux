#include "bootloader.h"

EFI_HANDLE imghdl;
EFI_SYSTEM_TABLE *systab;

EFI_STATUS EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	imghdl = ImageHandle;
	systab = SystemTable;
	systab->ConOut->ClearScreen(systab->ConOut);
	systab->ConOut->OutputString(systab->ConOut, L"Kernel: Hello World!");
	while (1);
	return EFI_SUCCESS;
}
