.section .text
	.align 4
	.global CPU_CONTEXT_SWITCH

CPU_CONTEXT_SWITCH:

	/*
	 * Saving all registers
	 * All register are relevant here
	 * special registers in compiling conventions are :
	 * r1, r5, r6, r7, r8, r14, r15
	 * r14 and r15 need special care, since they are return registers,
	 * and could lead to stack overflow or unwanted behavior if mishandled
	 * r0 is a zero constant, it does not need to be stored
	 */

	swi r1,r5,0
	swi r2,r5,4
	swi r3,r5,8
	swi r4,r5,12
	swi r5,r5,16
	swi r6,r5,20
	swi r7,r5,24
	swi r8,r5,28
	swi r9,r5,32
	swi r10,r5,36
	swi r11,r5,40
	swi r12,r5,44
	swi r13,r5,48
	swi r14,r5,52
	swi r15,r5,56
	swi r16,r5,60
	swi r17,r5,64
	swi r18,r5,68
	swi r19,r5,72
	swi r20,r5,76
	swi r21,r5,80
	swi r22,r5,84
	swi r23,r5,88
	swi r24,r5,92
	swi r25,r5,96
	swi r26,r5,100
	swi r27,r5,104
	swi r28,r5,108
	swi r29,r5,112
	swi r30,r5,116
	swi r31,r5,120

	/*
   * Need to save the proc state, with instruction mfs
	 */

	mfs r2,rmsr
	swi r2,r5,124

	/* 
   * Old thread context saved, now time to restore next thread context
   * First, we push the location of the new thread context in the stack
   * then we start restoring all the registers of the next thread
   * The problem is what are we going to do with r15, the return
   * address register 
   * We can't restore it here, since it will force the thread to start once more 
   * as soon as commute end, and not in its original state, since it will
   * lead it to abruptly end the current procedure
   * Yet r5 is not used or usable anymore. It has been used to call commute
   * with arguments
   * In the context to be, it represents the position of the loading
   * thread context.
   * Since it is useless, we use it to restore r15 and to store the current 
   * return address value
	 */

	lwi r1,r6,0
	lwi r3,r6,8
	lwi r4,r6,12
	lwi r5,r6,16
	lwi r7,r6,24
	lwi r8,r6,28
	lwi r9,r6,32
	lwi r10,r6,36
	lwi r11,r6,40
	lwi r12,r6,44
	lwi r13,r6,48
	lwi r14,r6,52
	lwi r15,r6,56
	lwi r16,r6,60
	lwi r17,r6,64
	lwi r18,r6,68
	lwi r19,r6,72
	lwi r20,r6,76
	lwi r21,r6,80
	lwi r22,r6,84
	lwi r23,r6,88
	lwi r24,r6,92
	lwi r25,r6,96
	lwi r26,r6,100
	lwi r27,r6,104
	lwi r28,r6,108
	lwi r29,r6,112
	lwi r30,r6,116
	lwi r31,r6,120
	
	lwi r2,r6,124
	mts rmsr,r2
	lwi r2,r6,4

	/*
   * now that restoration of all register safe r6 is done, we can return and
   * use the pipeline latency to restore r6
	 */

	rtsd r15,8
	lwi r6,r6,20
