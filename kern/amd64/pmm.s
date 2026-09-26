BITS 64

section .text

pmm_set_pml4:
	mov cr3, rdi
	ret

pmm_get_pml4:
	mov rax, cr3
	mov [rdi], rax
	ret

global pmm_set_pml4
global pmm_get_pml4
