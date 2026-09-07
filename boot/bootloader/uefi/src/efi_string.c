#include "efi_string.h"

void *
efi_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *tmp_dst = (unsigned char *)dst;
	const unsigned char *tmp_src = (const unsigned char *)src;
	while (n > 0) {
		*tmp_dst++ = *tmp_src++;
		n--;
	}
	return dst;
}

void *
efi_memset(void *s, int c, size_t n)
{
	unsigned char *tmp_s = (unsigned char *)s;
	while (n > 0) {
		*tmp_s++ = (unsigned char)c;
		n--;
	}
	return s;
}

int
efi_memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *tmp_s1 = (const unsigned char *)s1;
	const unsigned char *tmp_s2 = (const unsigned char *)s2;
	while (n > 0) {
		if ((*tmp_s1 - *tmp_s2) != 0)
			return (int)(*tmp_s1 - *tmp_s2);
		tmp_s1++;
		tmp_s2++;
		n--;
	}
	return 0;
}

size_t
efi_strlen(const char *s)
{
	size_t len = 0;
	while (*s++ != '\0')
		len++;
	return len;
}
