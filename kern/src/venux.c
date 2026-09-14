#include "venux.h"

int a;
int b = 0;

void
kern_main(struct kern_args *kargs)
{
	int c = b;
	a = c;
	return;
}
