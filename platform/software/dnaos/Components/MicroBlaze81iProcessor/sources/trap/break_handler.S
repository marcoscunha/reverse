.section .text
	.align 4
	.global _break_handler

_break_handler:
	bri _break_handler
