BITS 64

extern page_tables

section .text

efi_fetch_tables:
	mov rbx, cr3
	mov [page_tables], rbx
	mov rax, 0
	ret

efi_apply_tables:
	mov rbx, [page_tables]
	mov cr3, rbx
	mov rax, 0
	ret

global efi_fetch_tables
global efi_apply_tables
