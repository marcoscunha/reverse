.section .text
	.align 4
  .global  _interrupt_handler
	.extern handler_table

_interrupt_handler:
	
	/* Registers not to be saved :
	 *  r0, r1, r2 and r13 are not supposed to be changed by anything called
	 *  from here, and ear, esr and fsr are not concerned by interrupt,
	 *  as far as I can say
	 *  This leaves out 29 registers in the context
	 *  Note that this context is not related to the thread context !
	 */
	
	addik r1,  r1, 4 * 30
	swi   r3,  r1, 4 * 0
	swi   r4,  r1, 4 * 1
	swi   r5,  r1, 4 * 2
	swi   r6,  r1, 4 * 3
	swi   r7,  r1, 4 * 4
	swi   r8,  r1, 4 * 5
	swi   r9,  r1, 4 * 6
	swi   r10, r1, 4 * 7
	swi   r11, r1, 4 * 8
	swi   r12, r1, 4 * 9
	swi   r14, r1, 4 * 10
	swi   r15, r1, 4 * 11
	swi   r16, r1, 4 * 12
	swi   r17, r1, 4 * 13
	swi   r18, r1, 4 * 14
	swi   r19, r1, 4 * 15
	swi   r20, r1, 4 * 16
	swi   r21, r1, 4 * 17
	swi   r22, r1, 4 * 18
	swi   r23, r1, 4 * 19
	swi   r24, r1, 4 * 20
	swi   r25, r1, 4 * 21
	swi   r26, r1, 4 * 22
	swi   r27, r1, 4 * 23
	swi   r28, r1, 4 * 24
	swi   r29, r1, 4 * 25
	swi   r30, r1, 4 * 26
	swi   r31, r1, 4 * 27
	mfs   r3,  rmsr
	swi   r3,  r1, 4 * 28
	
	/*
	 * Access to the isr table, that contains indeed a single element!
	 */
	
	addik  r3, r0, handler_table
	lw     r4, r0, r3
	addik  r3, r0, 4
	lw     r5, r0, r3
	brald  r15, r4
	
	/*
	 * Because we shall not have here an instruction that can
	 * generate an exception (a lw can), we have nothing interesting to do
	 * in this delay slot
	 */
	
	nop
	
	lwi   r3,  r1, 4 * 28
	mts   rmsr,  r3
	lwi   r3,  r1, 4 * 0
	lwi   r4,  r1, 4 * 1
	lwi   r5,  r1, 4 * 2
	lwi   r6,  r1, 4 * 3
	lwi   r7,  r1, 4 * 4
	lwi   r8,  r1, 4 * 5
	lwi   r9,  r1, 4 * 6
	lwi   r10, r1, 4 * 7
	lwi   r11, r1, 4 * 8
	lwi   r12, r1, 4 * 9
	lwi   r14, r1, 4 * 10
	lwi   r15, r1, 4 * 11
	lwi   r16, r1, 4 * 12
	lwi   r17, r1, 4 * 13
	lwi   r18, r1, 4 * 14
	lwi   r19, r1, 4 * 15
	lwi   r20, r1, 4 * 16
	lwi   r21, r1, 4 * 17
	lwi   r22, r1, 4 * 18
	lwi   r23, r1, 4 * 19
	lwi   r24, r1, 4 * 20
	lwi   r25, r1, 4 * 21
	lwi   r26, r1, 4 * 22
	lwi   r27, r1, 4 * 23
	lwi   r28, r1, 4 * 24
	lwi   r29, r1, 4 * 25
	lwi   r30, r1, 4 * 26
	lwi   r31, r1, 4 * 27
	rtid  r14, 0
	addik r1,  r1, -4 * 29
