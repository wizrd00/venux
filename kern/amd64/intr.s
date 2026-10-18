BITS 64

section .text

intr_disable:
	cli
	ret

intr_enable:
	sti
	ret

global intr_disable
global intr_enable
