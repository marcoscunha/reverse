.section .reset,"ax",@progbits
	.align 4
	.global _reset_handler
	.extern CPU_OS_ENTRY_POINT
	.extern CPU_SVC_STACK_ADDR

_reset_handler:

	/*
	 * Enable Caches
	 */
	
	ori     r3, r0, 0xA0 
	mts     rmsr, r3
	addik   r1, r0, CPU_SVC_STACK_ADDR
	bri     CPU_OS_ENTRY_POINT

_reset_end:
								bri 	_reset_end

