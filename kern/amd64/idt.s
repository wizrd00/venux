BITS 64

section .data

idtr:
	dw 0
	dq 0

section .text

idt_reload_table:
	mov [idtr], si
	mov [idtr + 2], rdi
	lidt [idtr]
	ret

global idt_reload_table
