#ifndef _KERN_ERRORS_H
#define _KERN_ERRORS_H

#include "kern_panic.h"

#define KERN_ERROR_INVALID_ARENA_SIZE 1
#define KERN_ERROR_OUT_OF_ARENA 2
#define KERN_ERROR_INVALID_MEMMAP 3
#define KERN_ERROR_PAGE_ALREADY_PRESENT 4
#define KERN_ERROR_INVALID_KERNEL_RANGE 5

#define RET_ERROR(_ret) (_ret != 0)

#define KERN_PANIC(_ret)\
	do {kern_panic(_ret, __LINE__);} while (0)

#define KERN_CRITICAL_ERROR(_ret, ...)\
	do {\
		/* TODO */\
		kern_panic(_ret, __LINE__);\
	} while (0)

#endif
