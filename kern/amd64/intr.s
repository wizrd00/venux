BITS 64

extern intr_isr_glob

%macro ISR_STUB 1
%if (%1 == 8) || (%1 >= 10 && %1 <= 14) || (%1 == 17) || (%1 == 21)
%else
	push 0
%endif
	push %1
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rbp
	push rsi
	push rdi
	push rdx
	push rcx
	push rbx
	push rax
	mov rdi, rsp
	call intr_isr_glob
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rdi
	pop rsi
	pop rbp
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	add rsp, 16
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
