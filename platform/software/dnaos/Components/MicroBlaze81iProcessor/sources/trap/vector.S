.section .excep,"ax",@progbits
	.align 4
	.global _start
	.extern _reset_handler
	.extern _exception_handler
	.extern _interrupt_handler
	.extern _break_handler
	.extern _hw_exception_handler

.org 0x0
_start:
	bri _reset_handler

.org 0x08
_exception_vector:
	bri _exception_handler

.org 0x10
_interrupt_vector:
	bri _interrupt_handler

.org 0x18
_break_vector:
	bri _break_handler

.org 0x20
_hw_exception_vector:
	bri _hw_exception_handler
