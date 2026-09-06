.PHONY: bootloader clean-bootloader

BOOTLOADER_NAME := BOOTX64.efi
BOOTLOADER_EXE := $(BOOTLOADER_NAME)
BOOTLOADER_SRC := $(wildcard boot/bootloader/uefi/src/*.c)
BOOTLOADER_HDR := $(wildcard boot/bootloader/uefi/include/*.h)
BOOTLOADER_ASM := $(wildcard boot/bootloader/uefi/amd64/*.s)
BOOTLOADER_OBJ := $(patsubst boot/bootloader/uefi/src/%.c, $(BIN_DIR)/%.c.o, $(BOOTLOADER_SRC)) $(patsubst boot/bootloader/uefi/amd64/%.s, $(BIN_DIR)/%.s.o, $(BOOTLOADER_ASM))
BOOTLOADER_CFLAGS_FINALL := -target x86_64-unknown-windows \
	-nostdlib \
	-mno-red-zone \
	-Wl,-entry:efi_main \
	-Wl,-subsystem:efi_application \
	-fuse-ld=lld-link

BOOTLOADER_CFLAGS := -c \
	-Iboot/bootloader/uefi/include \
	-Iboot/bootloader/uefi/include/UEFI \
	-target x86_64-unknown-windows \
	-nostdlib \
	-mno-red-zone \
	-fuse-ld=lld-link

BOOTLOADER_SFLAGS := $(patsubst elf64, win64, $(SFLAGS))

bootloader : $(BOOTLOADER_EXE)

$(BOOTLOADER_EXE) : $(BOOTLOADER_OBJ)
	$(CC) $(CFLAGS) $(BOOTLOADER_CFLAGS_FINALL) -o $@ $^

$(BIN_DIR)/%.c.o : boot/bootloader/uefi/src/%.c boot/bootloader/uefi/include/%.h
	$(CC) $(CFLAGS) $(BOOTLOADER_CFLAGS) -o $@ $<

$(BIN_DIR)/%.s.o : boot/bootloader/uefi/amd64/%.s
	$(AS) $(BOOTLOADER_SFLAGS) -o $@ $<

clean-bootloader :
	rm BOOTX64.*
