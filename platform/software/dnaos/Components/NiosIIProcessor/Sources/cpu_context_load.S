.section .text
	.align 4
	.global cpu_context_load

.set noat
.set nobreak

cpu_context_load:
	/*************************************************************
	save general purpose registers
	**************************************************************/
	stw	zero,	 0(r4)	            	
	stw	at,	 4(r4)		               
	stw	r2,	 8(r4)		              
	stw	r3,	 12(r4)	
	stw	r4,	 16(r4)	
	stw	r5,	 20(r4)	
	stw	r6,	 24(r4)	
	stw	r7,	 28(r4)	
	stw	r8,	 32(r4)	
	stw	r9,	 36(r4)	
	stw	r10,	 40(r4)	
	stw	r11,	 44(r4)	
	stw	r12,	 48(r4)	
	stw	r13,	 52(r4)	
	stw	r14,	 56(r4)	
	stw	r15,	 60(r4)	
	stw	r16,	 64(r4)		            
	stw	r17,	 68(r4)		            
	stw	r18,	 72(r4)		            
	stw	r19,	 76(r4)		            
	stw	r20,	 80(r4)		            
	stw	r21,	 84(r4)		            
	stw	r22,	 88(r4)		            
	stw	r23,	 92(r4)		            
	stw	et,	 96(r4)		            
	stw	bt,	 100(r4)	                
	stw	gp,	 104(r4)	                
	stw	sp,	 108(r4)	                
	stw	fp,	 112(r4)	                
	stw	ea,	 116(r4)	                
	stw	ba,	 120(r4)	                
	stw	ra,	 124(r4)
	/*************************************************************
	save status and ienable registers
	**************************************************************/
	rdctl	r16,	 status
	rdctl	r17,	 ienable
	stw	r16,	 128(r4)
	stw	r17,	 132(r4)	
	/*************************************************************
	restore the saved registers
	**************************************************************/
	ldw	at,	 4(r5)	
	ldw	r2,	 8(r5)	
	ldw	r3,	 12(r5)	
	ldw	r4,	 16(r5)	
	ldw	r6,	 24(r5)	
	ldw	r7,	 28(r5)	
	ldw	r8,	 32(r5)	
	ldw	r9,	 36(r5)	
	ldw	r10,	 40(r5)	
	ldw	r11,	 44(r5)	
	ldw	r12,	 48(r5)	
	ldw	r13,	 52(r5)	
	ldw	r14,	 56(r5)	
	ldw	r15,	 60(r5)
	/*************************************************************
	restore the status and ienable registers
	**************************************************************/
	ldw	r16,	 128(r5)	                
	ldw	r17,	 132(r5)	                
	wrctl	status,	 r16
	wrctl	ienable, r17
	ldw	r16,	 64(r5)
	ldw	r17,	 68(r5)		            
	ldw	r18,	 72(r5)		            
	ldw	r19,	 76(r5)		            
	ldw	r20,	 80(r5)		            
	ldw	r21,	 84(r5)		            
	ldw	r22,	 88(r5)		            
	ldw	r23,	 92(r5)		            
	ldw	bt,	 100(r5)	                
	ldw	gp,	 104(r5)	                
	ldw	sp,	 108(r5)	                
	ldw	fp,	 112(r5)	                
	ldw	ea,	 116(r5)	                
	ldw	ba,	 120(r5)	                
	ldw	ra,	 124(r5)	                
	jmp	ra
	nop
	nop
		
_ctx_init:
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

_ctx_load:
	/*************************************************************
	restore the saved registers
	**************************************************************/
	ldw	at,	 4(r4)	
	ldw	r2,	 8(r4)	
	ldw	r3,	 12(r4)	
	ldw	r5,	 16(r4)	
	ldw	r6,	 24(r4)	
	ldw	r7,	 28(r4)	
	ldw	r8,	 32(r4)	
	ldw	r9,	 36(r4)	
	ldw	r10,	 40(r4)	
	ldw	r11,	 44(r4)	
	ldw	r12,	 48(r4)	
	ldw	r13,	 52(r4)	
	ldw	r14,	 56(r4)	
	ldw	r15,	 60(r4)
	/*************************************************************
	restore the status and ienable registers
	**************************************************************/
	ldw	r16,	 128(r4)	                
	ldw	r17,	 132(r4)	                
	wrctl	status,	 r16
	wrctl	ienable, r17	
	ldw	r16,	 64(r4)
	ldw	r17,	 68(r4)		            
	ldw	r18,	 72(r4)		            
	ldw	r19,	 76(r4)		            
	ldw	r20,	 80(r4)		            
	ldw	r21,	 84(r4)		            
	ldw	r22,	 88(r4)		            
	ldw	r23,	 92(r4)		            
	ldw	bt,	 100(r4)	                
	ldw	gp,	 104(r4)	                
	ldw	sp,	 108(r4)	                
	ldw	fp,	 112(r4)	                
	ldw	ea,	 116(r4)	                
	ldw	ba,	 120(r4)	                
	ldw	ra,	 124(r4)	                
	jmp	ra
	nop
	nop

_ctx_save:
	/*************************************************************
	save general purpose registers
	**************************************************************/
	stw	zero,	 0(r4)	            	
	stw	at,	 4(r4)		               
	stw	r2,	 8(r4)		              
	stw	r3,	 12(r4)	
	stw	r4,	 16(r4)	
	stw	r5,	 20(r4)	
	stw	r6,	 24(r4)	
	stw	r7,	 28(r4)	
	stw	r8,	 32(r4)	
	stw	r9,	 36(r4)	
	stw	r10,	 40(r4)	
	stw	r11,	 44(r4)	
	stw	r12,	 48(r4)	
	stw	r13,	 52(r4)	
	stw	r14,	 56(r4)	
	stw	r15,	 60(r4)	
	stw	r16,	 64(r4)		            
	stw	r17,	 68(r4)		            
	stw	r18,	 72(r4)		            
	stw	r19,	 76(r4)		            
	stw	r20,	 80(r4)		            
	stw	r21,	 84(r4)		            
	stw	r22,	 88(r4)		            
	stw	r23,	 92(r4)		            
	stw	et,	 96(r4)		            
	stw	bt,	 100(r4)	                
	stw	gp,	 104(r4)	                
	stw	sp,	 108(r4)	                
	stw	fp,	 112(r4)	                
	stw	ea,	 116(r4)	                
	stw	ba,	 120(r4)	                
	stw	ra,	 124(r4)
	/*************************************************************
	save status and ienable registers
	**************************************************************/
	rdctl	r16,	 status
	rdctl	r17,	 ienable
	stw	r16,	 128(r4)
	stw	r17,	 132(r4)
	ldw	r16,	 64(r4)
	ldw	r17,	 68(r4)		            
	jmp	ra
	nop
	nop

.set at 
.set break
