BITS 64
default rel

section .text

pmm_set_pml4:
	mov cr3, rdi
	ret

pmm_get_pml4:
	mov rax, cr3
	mov [rdi], rax
	ret

pmm_set_efer_nxe:
	mov ecx, 0xc0000080
	rdmsr
	or eax, (1 << 11)
	wrmsr
	ret

global pmm_set_pml4
global pmm_get_pml4
global pmm_set_efer_nxe
