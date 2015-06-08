.section ".reset", "ax"
	.align 4
	.global _start

	.extern CPU_OS_ENTRY_POINT
	.extern CPU_SVC_STACK_ADDR
	.extern _gp

_start:
	br	_reset_handler

_reset_handler:
	/*************************************************************
	initialize the default stack
	**************************************************************/
	movia	sp, CPU_SVC_STACK_ADDR

	/* setup global data pointer */
	movia     gp,     _gp 
	/*************************************************************
	address of the startup routine
	**************************************************************/
	movia	r17, CPU_OS_ENTRY_POINT
	jmp r17


