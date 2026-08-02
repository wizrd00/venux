.PHONY: bootloader clean

CC := clang
LD := ld.lld

OPTIMIZATION_LEVEL := -O0 #temporary
CFLAGS := \
	${OPTIMIZATION_LEVEL} \
	-std=c99 \
	-ffreestanding \
	-Werror \
	-Wall \
	-Wextra

BIN_DIR := bin
SRC_DIR := src
HDR_DIR := include
LIB_DIR := lib

# __________BOOTLOADER__________

BOOTLOADER_NAME := BOOTX64.efi
BOOTLOADER_EXE := ${BOOTLOADER_NAME}
BOOTLOADER_OBJ := ${BIN_DIR}/bootloader.o
BOOTLOADER_SRC := ${SRC_DIR}/bootloader/bootloader.c
BOOTLOADER_HDR := ${HDR_DIR}/bootloader/bootloader.h
BOOTLOADER_CFLAGS := \
	-I${HDR_DIR}/bootloader/ \
	-I${HDR_DIR}/bootloader/UEFI/ \
	-target x86_64-unknown-windows \
	-nostdlib \
	-mno-red-zone \
	-Wl,-entry:efi_main \
	-Wl,-subsystem:efi_application \
	-fuse-ld=lld-link-21

bootloader : ${BOOTLOADER_EXE}

${BOOTLOADER_EXE} : ${BOOTLOADER_SRC} ${BOOTLOADER_HDR}
	${CC} ${CFLAGS} ${BOOTLOADER_CFLAGS} -o ${BOOTLOADER_EXE} \
	    ${BOOTLOADER_SRC}

${BIN_DIR} :
	mkdir -p ${BIN_DIR}

clean :
	rm ${BIN_DIR}/*
