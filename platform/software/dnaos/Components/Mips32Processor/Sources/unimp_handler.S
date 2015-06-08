.section .text
	.align 4
	.global Uimp

.ent Uimp
Uimp  : 
	j Uimp
.end Uimp
