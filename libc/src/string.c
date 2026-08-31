#include "string.h"

void *
memcpy(void *restrict dst, const void *restrict src, size_t n)
{
	unsigned char *restrict tmp_dst = (unsigned char *)dst;
	const unsigned char *restrict tmp_src = (const unsigned char *)src;
	while (n > 0) {
		*tmp_dst++ = *tmp_src++;
		n--;
	}
	return dst;
}

void *
memset(void *s, int c, size_t n)
{
	unsigned char *tmp_s = (unsigned char *)s;
	while (n > 0) {
		*tmp_s++ = (unsigned char)c;
		n--;
	}
	return s;
}
