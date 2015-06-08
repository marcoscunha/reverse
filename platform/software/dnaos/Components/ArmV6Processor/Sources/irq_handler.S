.section .excep,#alloc,#execinstr
	.align 4
	.global irq_handler
  .extern PLATFORM_AICU_BASE

irq_registers:
  .word 0x0 @ R1
  .word 0x0 @ LR
  .word 0x0 @ CPSR

irq_handler:
	@ Save scratch registers and SPSR
	push {r0}
  ldr r0, =irq_registers
  stmia r0!, {r1, lr}
	mrs	lr, spsr
  str lr, [r0]
  pop {r0}

	@ Move to SVC mode
	mrs lr, cpsr
	orr lr, lr, #0xC3
	msr cpsr, lr 

  @ Save the context
  push {r0-r3, r9-r12, lr}
  ldr r0, =irq_registers
  ldmia r0, {r1, r2, r3}
  push {r1, r2, r3}

	@ Fetch the current interruption source
	@ Branch to IRQ handler

	ldr r1, =PLATFORM_AICU_BASE
	ldr r1, [r1]
  mrc p15,0,r2,c0,c0,5
  add r1, r1, #0x100
  add r1, r1, r2, lsl #0x4

	ldr r0, [r1, #0xC]
	ldr r1, [r1, #0x8]
	blx r1

	@ Restore the context
  ldr r0, =irq_registers
  pop {r1, r2, r3}
  stmia r0, {r1, r2, r3}
  pop {r0-r3, r9-r12, lr}
  
	@ Move back to IRQ mode
	mrs r1, cpsr
	bic r1, r1, #0x1
	msr cpsr, r1

	@ Restore registers
  push {r0, r2}
  ldr r0, =irq_registers
  ldmia r0!, {r1, lr}
  ldr r2, [r0]
	msr	spsr, r2
  pop {r0, r2}

	@ Return
	subs pc, lr, #0x4
  nop
  nop
