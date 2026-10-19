BITS 64

INTR_DE equ 0
INTR_DB equ 1
INTR_NMI equ 2
INTR_BP equ 3
INTR_OF equ 4
INTR_BR equ 5
INTR_UD equ 6
INTR_NM equ 7
INTR_DF equ 8
INTR_TS equ 10
INTR_NP equ 11
INTR_SS equ 12
INTR_GP equ 13
INTR_PF equ 14
INTR_MF equ 16
INTR_AC equ 17
INTR_MC equ 18
INTR_XM equ 19
INTR_VE equ 20
INTR_CP equ 21

extern intr_isr_glob

section .text

intr_disable:
	cli
	ret

intr_enable:
	sti
	ret

intr_isr_de:
	mov rdi, INTR_DE
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_db:
	mov rdi, INTR_DB
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_nmi:
	mov rdi, INTR_NMI
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_bp:
	mov rdi, INTR_BP
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_of:
	mov rdi, INTR_OF
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_br:
	mov rdi, INTR_BR
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_ud:
	mov rdi, INTR_UD
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_nm:
	mov rdi, INTR_NM
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_df:
	mov rdi, INTR_DF
	pop rsi
	call intr_isr_glob
	iretq

intr_isr_ts:
	mov rdi, INTR_TS
	pop rsi
	call intr_isr_glob
	iretq

intr_isr_np:
	mov rdi, INTR_NP
	pop rsi
	call intr_isr_glob
	iretq

intr_isr_ss:
	mov rdi, INTR_SS
	pop rsi
	call intr_isr_glob
	iretq

intr_isr_gp:
	mov rdi, INTR_GP
	pop rsi
	call intr_isr_glob
	iretq

intr_isr_pf:
	mov rdi, INTR_PF
	pop rsi
	call intr_isr_glob
	iretq

intr_isr_mf:
	mov rdi, INTR_MF
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_ac:
	mov rdi, INTR_AC
	pop rsi
	call intr_isr_glob
	iretq

intr_isr_mc:
	mov rdi, INTR_MC
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_xm:
	mov rdi, INTR_XM
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_ve:
	mov rdi, INTR_VE
	mov rsi, 0
	call intr_isr_glob
	iretq

intr_isr_cp:
	mov rdi, INTR_CP
	pop rsi
	call intr_isr_glob
	iretq

global intr_disable
global intr_enable
global intr_isr_de
global intr_isr_db
global intr_isr_nmi
global intr_isr_bp
global intr_isr_of
global intr_isr_br
global intr_isr_ud
global intr_isr_nm
global intr_isr_df
global intr_isr_ts
global intr_isr_np
global intr_isr_ss
global intr_isr_gp
global intr_isr_pf
global intr_isr_mf
global intr_isr_ac
global intr_isr_mc
global intr_isr_xm
global intr_isr_ve
global intr_isr_cp
