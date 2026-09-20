BITS 64

section .text

kern_set_pml4:
	mov cr3, rdi
	ret
