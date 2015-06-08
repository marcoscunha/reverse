#define FIQ_MODE 0xD1
#define IRQ_MODE 0xD2
#define SVC_MODE 0xD3
#define USR_MODE 0xD0

.section .excep,#alloc,#execinstr
	.align 4
	.global reset_handler
	.extern system_kickstart
	.extern CPU_SVC_STACK_ADDR

.type   reset_handler, %function

reset_handler:
  @ Get the current cpuid
  mrc p15, 0, r0, c0, c0, 5
  mov r3, #0x400

  @ Compute the basic stack addresses
	ldr	r1, = CPU_SVC_STACK_ADDR
  mul r2, r3, r0
  sub r1, r1, r2

  # eight-byte stack alignment
  bic r1, r1, #7

  @ Set the processor stack
	mov	r0, #FIQ_MODE
	msr	cpsr_c, r0
	mov	sp, r1
	mov fp, sp

	mov	r0, #IRQ_MODE
	msr	cpsr_c, r0
	mov	sp, r1
	mov fp, sp

	mov	r0,  #SVC_MODE
	msr cpsr_c, r0
	mov	sp, r1
	mov fp, sp

	@ Enable CPU interrupts
	mrs r0, cpsr
	bic r0, r0, #0xC0
	msr cpsr_c, r0

	@ Call to system kickstart
	ldr	r0, = system_kickstart
  blx r0

  nop
  nop

_hell_pit:
  b _hell_pit

.size reset_handler, .-reset_handler