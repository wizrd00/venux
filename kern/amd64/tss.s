BITS 64

section .text

tss_reload_table:
	mov ax, di
	ltr ax
	ret

global tss_reload_table
