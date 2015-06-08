.section .text
	.align 4
	.global CPU_CONTEXT_SAVE

CPU_CONTEXT_SAVE:

	/*
	 * Saving all registers
	 * All register are relevant here
	 * special registers in compiling conventions are :
	 * r1, r5, r6, r7, r8, r14, r15
	 * r14 and r15 need special care, since they are return registers, and
	 * could lead to stack overflow or unwanted behavior if mishandled
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
	rtsd r15,8
	swi r2,r5,124
