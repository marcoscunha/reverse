.section .text
	.align 4
	.global cpu_context_save

.set noat
.set nobreak

cpu_context_save:
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
  /*************************************************************
  in the new API, r5 contains the address where we need to
  branch on return
  **************************************************************/
	stw	r5,	 124(r4)
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
