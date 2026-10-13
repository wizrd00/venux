BITS 64

section .data

idtr:
	dw 0
	dq 0

section .text

idt_set_table:
	mov [idtr], si
	mov [idtr + 2], rdi
	ret

global idt_set_table
