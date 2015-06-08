.section .text
	.align 4
	.global cpu_power_wake_on_interrupt

.ent cpu_power_wake_on_interrupt
cpu_power_wake_on_interrupt :
  wait
	nop
  j $31
  nop
.end cpu_power_wake_on_interrupt
