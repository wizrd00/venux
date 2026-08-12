.PHONY: bootloader clean

CC := clang
PATH != pwd

BIN_DIR := bin
SRC_DIR := src
HDR_DIR := include
LIB_DIR := lib


OPTIMIZATION_LEVEL := -O0 #temporary
CFLAGS := \
	$(OPTIMIZATION_LEVEL) \
	-std=c99 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-pic \
	-fno-pie \
	-mno-red-zone \
	-mno-sse \
	-mno-mmx \
	-Werror \
	-Wall \
	-Wextra

include boot/bootloader/uefi/makefile
include libc/makefile

# TODO : index sources that need to compile saperately and link to make kernel
$(BIN_DIR) :
	mkdir -p $(BIN_DIR)

clean :
	rm $(BIN_DIR)/*
