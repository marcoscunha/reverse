.section .text
	.align 4
	.global DBE

.ent DBE
DBE :
	j DBE
.end DBE
