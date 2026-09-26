BITS 64

INVALID_BOUNDARY_1 equ 1
INVALID_BOUNDARY_2 equ 2
INVALID_BOUNDARY_3 equ 3

extern _kernel_base
extern _kernel_gap
extern _kernel_start
extern _kernel_end
extern _kernel_arena_size
extern _kernel_arena_start
extern _kernel_arena_end
extern _kernel_stack_size
extern _kernel_stack_start
extern _kernel_stack_end
extern kern_main

section .text

_halt:
	hlt
	jmp _halt

kern_panic:
	cli
	jmp _halt

kern_init:
	cli
	mov rax, _kernel_base
	add rax, _kernel_gap
	cmp rax, _kernel_start
	mov rdi, INVALID_BOUNDARY_1
	mov rsi, 0
	jne kern_panic
	mov rax, _kernel_arena_start
	add rax, _kernel_arena_size
	cmp rax, _kernel_arena_end
	mov rdi, INVALID_BOUNDARY_2
	mov rsi, 0
	jne kern_panic
	mov rax, _kernel_stack_start
	add rax, _kernel_stack_size
	cmp rax, _kernel_stack_end
	mov rdi, INVALID_BOUNDARY_3
	mov rsi, 0
	jne kern_panic
	mov rdi, rsp
	mov rsp, _kernel_stack_end
	call kern_main
	mov rdi, 0
	mov rsi, 0
	jmp kern_panic

global kern_init
global kern_panic
