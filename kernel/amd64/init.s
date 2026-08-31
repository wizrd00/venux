BITS 64

extern _kernel_base
extern _kernel_gap
extern _kernel_size
extern _kernel_start
extern _kernel_end
extern _kernel_stack_size
extern _kernel_stack_start
extern _kernel_stack_end
extern kern_main

global _init

section .text

_halt:
	hlt
	jmp _halt

_panic:
	cli
	jmp _halt

_init:
	cli
	mov rax, _kernel_base
	add rax, _kernel_gap
	cmp rax, _kernel_end
	jne _panic
	mov rax, _kernel_start
	add rax, _kernel_size
	cmp rax, _kernel_end
	jne _panic
	mov rax, _kernel_stack_start
	add rax, _kernel_stack_size
	cmp rax, _kernel_stack_end
	jne _panic
	mov rax, _kernel_start
	cmp rax, _kernel_stack_end
	jne _panic
	mov rsp, _kernel_stack_end
	call kern_main
	jmp _panic
