BITS 64

extern pml4
extern kargs
extern kernel_entry
extern efi_halt

section .text

efi_leave:
	cli
	mov rax, [pml4]
	mov cr3, rax
	mov rsp, kargs
	jmp [kernel_entry]
	jmp efi_halt

global efi_leave
