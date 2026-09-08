BITS 64

extern pml4

section .text

efi_apply_tables:
	mov rbx, pml4
	mov rax, 0
	ret

global efi_apply_tables
