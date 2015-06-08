.section .text
	.align 4
	.global CPU_CONTEXT_LOAD

CPU_CONTEXT_LOAD:
	lwi r1,r5,0
	lwi r3,r5,8
	lwi r4,r5,12
	lwi r6,r5,20
	lwi r7,r5,24
	lwi r8,r5,28
	lwi r9,r5,32
	lwi r10,r5,36
	lwi r11,r5,40
	lwi r12,r5,44
	lwi r13,r5,48
	lwi r14,r5,52
	lwi r15,r5,56
	lwi r16,r5,60
	lwi r17,r5,64
	lwi r18,r5,68
	lwi r19,r5,72
	lwi r20,r5,76
	lwi r21,r5,80
	lwi r22,r5,84
	lwi r23,r5,88
	lwi r24,r5,92
	lwi r25,r5,96
	lwi r26,r5,100
	lwi r27,r5,104
	lwi r28,r5,108
	lwi r29,r5,112
	lwi r30,r5,116
	lwi r31,r5,120
	
	lwi r2,r5,124
	mts rmsr,r2
	lwi r2,r5,4

	/*
   * now that restoration of all register safe r6 is done, we can return
   * and use the pipeline latency to restore r6
	 */

	rtsd r15,8
	lwi r5,r5,20

