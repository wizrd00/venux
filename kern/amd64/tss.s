BITS 64

section .text

tss_set_table:
	ret

tss_reload_table:
	ret

global tss_set_table
global tss_reload_table
