BITS 64

global efi_halt

section .text

efi_halt:
	cli
	hlt
	jmp efi_halt
	ret
