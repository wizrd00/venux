BITS 64

INVALID_BOUNDARY_1 equ 1
INVALID_BOUNDARY_2 equ 2
INVALID_BOUNDARY_3 equ 3
INVALID_BOUNDARY_4 equ 4
KERN_MAIN_RETURNED equ 5

extern _kernel_base
extern _kernel_gap
extern _kernel_size
extern _kernel_start
extern _kernel_end
extern _kernel_stack_size
extern _kernel_stack_start
extern _kernel_stack_end
extern kern_main

global kern_init

section .text

_halt:
	hlt
	jmp _halt

_panic:
	cli
	jmp _halt

kern_init:
	cli
	mov rax, _kernel_base
	add rax, _kernel_gap
	cmp rax, _kernel_start
	mov rbx, INVALID_BOUNDARY_1
	jne _panic
	mov rax, _kernel_start
	add rax, _kernel_size
	cmp rax, _kernel_end
	mov rbx, INVALID_BOUNDARY_2
	jne _panic
	mov rax, _kernel_stack_start
	add rax, _kernel_stack_size
	cmp rax, _kernel_stack_end
	mov rbx, INVALID_BOUNDARY_3
	jne _panic
	mov rax, _kernel_start
	cmp rax, _kernel_stack_end
	mov rbx, INVALID_BOUNDARY_4
	jne _panic
	mov rsp, _kernel_stack_end
	call kern_main
	mov rbx, KERN_MAIN_RETURNED
	jmp _panic
