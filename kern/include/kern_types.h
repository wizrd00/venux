#ifndef _KERN_TYPES_H
#define _KERN_TYPES_H

struct kern_args {
	void *uefi_rt;
	void *acpi;
	void *gop;
};

#endif
