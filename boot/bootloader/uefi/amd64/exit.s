BITS 64

extern pml4

section .text

efi_exit:

efi_apply_tables:
	mov rbx, [pml4]
	mov cr3, rbx
	mov rax, 0
	cli
	hlt

global efi_exit
