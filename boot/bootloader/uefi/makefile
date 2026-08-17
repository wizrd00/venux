.PHONY: bootloader

BOOTLOADER_NAME := BOOTX64.efi
BOOTLOADER_EXE := $(BOOTLOADER_NAME)
BOOTLOADER_SRC := boot/bootloader/uefi/src/bootloader.c
BOOTLOADER_HDR := boot/bootloader/uefi/include/bootloader.h
BOOTLOADER_CFLAGS := \
	-Iboot/bootloader/uefi/include \
	-Iboot/bootloader/uefi/include/UEFI \
	-target x86_64-unknown-windows \
	-nostdlib \
	-mno-red-zone \
	-Wl,-entry:efi_main \
	-Wl,-subsystem:efi_application \
	-fuse-ld=lld-link

bootloader : $(BOOTLOADER_EXE)

$(BOOTLOADER_EXE) : $(BOOTLOADER_SRC) $(BOOTLOADER_HDR)
	$(CC) $(CFLAGS) $(BOOTLOADER_CFLAGS) -o $(BOOTLOADER_EXE) $(BOOTLOADER_SRC)
