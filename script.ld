STARTUP(bin/init.s.o)
ENTRY(kern_init)

_kernel_base = 0xffffffff80000000;
_kernel_gap = 1M;

_kernel_arena_size = 256K;
_kernel_stack_size = 4K;

PHDRS
{
	text PT_LOAD;
	rodata PT_LOAD;
	data PT_LOAD;
	bss PT_LOAD;
	arena PT_LOAD;
	stack PT_LOAD;
}

SECTIONS
{
	_kernel_start = _kernel_base + _kernel_gap;
	. = _kernel_start;
	.text : ALIGN(4K) {
		_kernel_text_start = .;
		*(.text*)
	} :text
	.rodata : ALIGN(4K) {
		_kernel_text_end = .;
		_kernel_rodata_start = .;
		*(.rodata*)
	} :rodata
	.data : ALIGN(4K) {
		_kernel_rodata_end = .;
		_kernel_data_start = .;
		*(.data*)
	} :data
	.bss : ALIGN(4K) {
		_kernel_data_end = .;
		_kernel_bss_start = .;
		*(.bss*)
	} :bss
	.arena : ALIGN(4K) {
		_kernel_bss_end = .;
		_kernel_arena_start = .;
		. += _kernel_arena_size;
	} :arena
	.stack : ALIGN(4K) {
		_kernel_arena_end = .;
		_kernel_stack_start = .;
		. += _kernel_stack_size;
	} :stack
	_kernel_stack_end = .;
	_kernel_end = .;
}

OUTPUT_FORMAT(elf64-x86-64)
