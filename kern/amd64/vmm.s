BITS 64
default rel

section .text

vmm_set_pml4:
	mov cr3, rdi
	ret

vmm_get_pml4:
	mov rax, cr3
	mov [rdi], rax
	ret

vmm_set_efer_nxe:
	mov ecx, 0xc0000080
	rdmsr
	or eax, (1 << 11)
	wrmsr
	ret

vmm_reload_tlb:
	mov cr3, cr3
	ret

global vmm_set_pml4
global vmm_get_pml4
global vmm_set_efer_nxe
global vmm_reload_tlb
