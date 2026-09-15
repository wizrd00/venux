#include "venux.h"

void
kern_main(struct kern_args *kargs)
{
	int ret = 0;
	ret = kern_alloc_init();
	if (RET_ERROR(ret))
		KERN_CRITICAL_ERROR("kern_alloc_init() failed");
	return;
}
