STARTUP(bin/init.s.o)
ENTRY(_init)

_kernel_base = 0xffffffff80000000;
_kernel_gap = 1M;

_kernel_stack_size = 4K;
_kernel_stack_start = _kernel_base + _kernel_gap - _kernel_stack_size;
_kernel_stack_end = _kernel_stack_start + _kernel_stack_size;

PHDRS
{
	kern PT_LOAD PHDRS;
}

SECTIONS
{
	_kernel_start = _kernel_base + _kernel_gap;
	. = _kernel_start;
	.text : ALIGN(4K) {
		_kernel_text_start = .;
		*(.text*)
		_kernel_text_end = .;
	} :kern
	.rodata : ALIGN(4K) {
		_kernel_rodata_start = .;
		*(.rodata*)
		_kernel_rodata_end = .;
	} :kern
	.data : ALIGN(4K) {
		_kernel_data_start = .;
		*(.data*)
		_kernel_data_end = .;
	} :kern
	.bss : ALIGN(4K) {
		_kernel_bss_start = .;
		*(.bss*)
		_kernel_bss_end = .;
	} :kern
	_kernel_end = .;
	_kernel_size = _kernel_end - _kernel_start;
}

OUTPUT_FORMAT(elf64-x86-64)
