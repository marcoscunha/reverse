.section .text
	.align 4
	.global cpu_context_init

.set noat
.set nobreak

cpu_context_init:
	/*************************************************************
	save general purpose registers
	16(r4) : copy of r4
	104(r4) : copy of gp
	108(r4) : copy of sp 
	112(r4) : copy of fp
	124(r4) : copy of ra
	**************************************************************/
	ldw r8, 0(sp)
	stw	r8,	 16(r4)	
	stw	gp,	 104(r4)	                
	add r5, r5, r6
	subi r5, r5, 4
	stw	r5,	 108(r4)	                
	stw r5,	 112(r4)	                
	stw	r7,	 124(r4)
	rdctl	r9,	 status
	rdctl	r10,	 ienable
	stw	r9,	 128(r4)
	stw	r10,	 132(r4)
	jmp	ra
	nop
	nop	

