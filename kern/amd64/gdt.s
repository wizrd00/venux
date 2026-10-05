BITS 64

section .data

gdtr:
	dw 0
	dq 0

section .text

gdt_set_table:
	mov [gdtr], si
	mov [gdtr + 2], rdi
	lgdt [gdtr]
	ret

reload_gdt:
	push 0x08
	lea rax, [rel .reload_cs]
	push rax
	retfq

.reload_cs:
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	ret

global gdt_set_table
