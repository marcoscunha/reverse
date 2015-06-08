.section .excep,#alloc,#execinstr
	.align 4
	.global fiq_handler

fiq_handler:
	@ Not Handled
	subs pc, lr, #0x4
  nop
  nop
