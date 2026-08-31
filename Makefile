.PHONY: all clean clean-all

CC := clang
AS := nasm
LD := ld
AR := ar
ARCH := amd64

BIN_DIR := bin
SRC_DIR := src
HDR_DIR := include

OPTIMIZATION_LEVEL := 0 #temporary
CFLAGS := -O$(OPTIMIZATION_LEVEL) \
	-g \
	-ggdb \
	-std=c99 \
	-nostdlib \
	-ffreestanding \
	-fshort-wchar \
	-fno-stack-protector \
	-fno-exceptions \
	-fno-threadsafe-statics \
	-fno-asynchronous-unwind-tables \
	-fno-pic \
	-fno-pie \
	-mno-red-zone \
	-mno-sse \
	-mno-mmx \
	-Werror \
	-Wall \
	-Wextra \
	-Wno-unused-function \

ALL_CFLAGS :=

SFLAGS := -O$(OPTIMIZATION_LEVEL) \
	-g \
	-f elf64

MODULES := bootloader kernel libc

include boot/bootloader/uefi/Makefile
include kernel/Makefile
include libc/Makefile

all : $(MODULES)
	$(LD) -T script.ld -no-pie -o venux.elf $(BIN_DIR)/*.a

$(BIN_DIR) :
	mkdir -p $(BIN_DIR)

clean :
	@echo "clean-all clean-bootloader clean-kernel"

clean-all : clean clean-bootloader clean-kernel
	rm $(BIN_DIR)/*
