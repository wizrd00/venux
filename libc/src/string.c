#include "string.h"

void *
memcpy(void *dst, const void *src, size_t len)
{
	unsigned char *_dst = (unsigned char *)dst;
	const unsigned char *_src = (unsigned char *)src;
	while (len--) {
		*_dst++ = *_src++;
	}
	return dst;
}
