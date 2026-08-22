.PHONY: bootloader clean

CC := clang

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
	-Wextra

include boot/bootloader/uefi/Makefile
include libc/Makefile

# TODO : index sources that need to compile saperately and link to make kernel
$(BIN_DIR) :
	mkdir -p $(BIN_DIR)

clean :
	rm $(BIN_DIR)/*
