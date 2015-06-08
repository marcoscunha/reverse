.section .text
	.align 4
  .global  _exception_handler

_exception_handler:
	bri _exception_handler
