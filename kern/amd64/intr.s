BITS 64

extern intr_isr_glob

%macro ISR_STUB 1
	mov rdi, %1
%if (%1 == 8) || (%1 >= 10 && %1 <= 14) || (%1 == 17) || (%1 == 21)
	pop rsi
%else
	mov rsi, 0
%endif
	call intr_isr_glob
	iretq
%endmacro

section .text

intr_disable:
	cli
	ret

intr_enable:
	sti
	ret

%assign i 0
%rep 256
intr_isr_%+i:
	ISR_STUB %[i]
%assign i i+1
%endrep

section .rodata
isr_table:
%assign i 0
%rep 256
	dq intr_isr_%+i
%assign i i+1
%endrep

global intr_disable
global intr_enable
global isr_table
