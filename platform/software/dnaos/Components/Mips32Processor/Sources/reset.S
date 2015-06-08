.section .reset,"ax",@progbits
	.align 4
	.global _start

	.extern system_kickstart

.ent _start
_start:
	.set noat
	.set noreorder
	
	/* set up IT disable and kernel mode */
  li    $26, 0x2000FF01
	mtc0 $26,$12
	
	/* set stack for each processor of a SMP config*/
  mfc0 $k0, $0
  li $k1, 0x400
	la $sp,    CPU_SVC_STACK_ADDR - 0x10
  mul $k0, $k0, $k1
  subu $sp, $sp, $k0

	/* Start the OS */
	la $27, system_kickstart
  jal $27
  nop
	
	.set at
	.set reorder
.end _start	
