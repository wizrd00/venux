#ifndef _BOOTLOADER_EFI_STRING_H
#define _BOOTLOADER_EFI_STRING_H

#include <stddef.h>

void *efi_memcpy(void *dst, const void *src, size_t n);

void *efi_memset(void *s, int c, size_t n);

int efi_memcmp(const void *s1, const void *s2, size_t n);

size_t efi_strlen(const char *s);

#endif
