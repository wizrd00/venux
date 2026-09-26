#ifndef _KERN_GLOBALS_H
#define _KERN_GLOBALS_H

#define KERN_NAME "venux"

#define PHYSMEM_OFFSET 0xffff800000000000ULL
#define KERNEL_OFFSET (kern_vaddr - kern_paddr)

#define PAGE_SIZE 0x1000
#define HUGE_PAGE_SIZE 0x200000

#define PAGE_ALIGNED(_addr) ((_addr & 0xfffULL) == 0)
#define HUGE_PAGE_ALIGNED(_addr) ((_addr & 0x1fffffULL) == 0)

extern uint64_t kern_paddr;
extern uint64_t kern_vaddr;
extern uint64_t kern_size;

#endif
