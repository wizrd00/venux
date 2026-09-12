BITS 64

extern pml4
extern kernel_entry
extern efi_halt

section .text

efi_leave:
	cli
	mov rax, [pml4]
	mov cr3, rax
	mov rsp, 0
	mov rbx, [kernel_entry]
	jmp rbx
	jmp efi_halt

global efi_leave
