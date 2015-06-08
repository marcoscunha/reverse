.section .excep,"ax",@progbits
	.align 4
	.global _excep

_excep:
	.set noat
	.set nobreak
	
	movia	r17, _excep
	jmp r17
	nop

