.section .excep,"ax",@progbits
	.align 4
	.global _excep
	.extern Int
  .extern Uimp
  .extern Uimp
  .extern Uimp
  .extern AdEL
  .extern AdES
  .extern IBE
  .extern DBE
  .extern Sys
  .extern Bp
  .extern RI
  .extern Ovf
  .extern Ukn
  .extern Ukn

.ent _excep
_excep:
	.set noat
	.set noreorder
	addiu  $29, $29, -(22 * 4)  /* Load kernel stack addr + stack update */
	sw     $1,   0*4($29)       /*  save all temporaries*/
	sw     $2,   1*4($29)       /*  others are saved when calling procedures*/
	sw     $3,   2*4($29)
	sw     $4,   3*4($29)
	sw     $5,   4*4($29)
	sw     $6,   5*4($29)
	sw     $7,   6*4($29)
	sw     $8,   7*4($29)
	sw     $9,   8*4($29)
	sw    $10,   9*4($29)
	sw    $11,  10*4($29)
	sw    $12,  11*4($29)
	sw    $13,  12*4($29)
	sw    $14,  13*4($29)
	sw    $15,  14*4($29)
	sw    $24,  15*4($29)
	sw    $25,  16*4($29)
	sw    $30,  17*4($29)
	sw    $31,  18*4($29)

	mfhi  $26
	sw    $26,  19*4($29)
	mflo  $26
	sw    $26,  20*4($29)

	mfc0  $26,      $14     /* load EPC into k0 (two cycles befor use)*/
	nop
	sw    $26,  21*4($29)   /* and store it to be recallable*/
	nop
	
	/* Check the cause of the arrival in this code */

_excep_begin:
	mfc0  $27, $13           /* load CAUSE into $27*/
	nop
	andi  $27, $27, 0x3c     /*  get the exception code (ExcCode)*/
	bnez  $27, _excep_other  /*  Test if it is an interrupt. If so*/
	nop
	la    $26, Int
	jal   $26
	nop
	b     _excep_return
	nop

_excep_other:
	 la    $26, _excep_jmptable
	 addu  $26, $26, $27
	 lw    $27, ($26)
	 jal   $27 
	 nop

_excep_return:
	lw     $1,    0*4($29)
	lw     $2,    1*4($29)
	lw     $3,    2*4($29)
	lw     $4,    3*4($29)
	lw     $5,    4*4($29)
	lw     $6,    5*4($29)
	lw     $7,    6*4($29)
	lw     $8,    7*4($29)
	lw     $9,    8*4($29)
	lw    $10,    9*4($29)
	lw    $11,   10*4($29)
	lw    $12,   11*4($29)
	lw    $13,   12*4($29)
	lw    $14,   13*4($29)
	lw    $15,   14*4($29)
	lw    $24,   15*4($29)
	lw    $25,   16*4($29)
	lw    $30,   17*4($29)
	lw    $31,   18*4($29)
	
	/* Take care of hi and lo also */
	lw    $26,   19*4($29)
	mthi  $26
	lw    $27,   20*4($29)
	mtlo  $27
	
	lw    $26,   21*4($29)   /* This is the epc of this context */
  nop
  mtc0  $26,  $14
  addiu $29,  $29,  22 * 4
	eret 
.end _excep

_excep_jmptable:
.type _excep_jmptable, @object
     .word   Int
     .word   Uimp
     .word   Uimp
     .word   Uimp
     .word   AdEL
     .word   AdES
     .word   IBE
     .word   DBE
     .word   Sys
     .word   Bp
     .word   RI
     .word   Uimp
     .word   Ovf
     .word   Ukn
     .word   Ukn
     .word   Ukn
   .set at
   .set reorder
.size _excep_jmptable, .-_excep_jmptable

