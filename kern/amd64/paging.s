BITS 64

section .text

kern_set_pml4:
	mov rax, rdi
	mov cr3, rax
	hlt

kern_get_pml4:
	mov rax, cr3
	mov [rdi], rax
	ret

global kern_set_pml4
global kern_get_pml4
