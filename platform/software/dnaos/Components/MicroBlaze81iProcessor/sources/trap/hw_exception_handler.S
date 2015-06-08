.section .text
	.align 4
  .global  _hw_exception_handler

_hw_exception_handler:
	bri _hw_exception_handler
