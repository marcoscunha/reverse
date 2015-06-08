.section .text
	.align 4
	.global cpu_context_load

.ent cpu_context_load
cpu_context_load:
	.set noat
	.set noreorder

  /*
   * Loading FP registers.
   */

	lwc1   $f0,       ($4)
	lwc1   $f1,    1*4($4)
	lwc1   $f2,    2*4($4)
	lwc1   $f3,    3*4($4)
	lwc1   $f4,    4*4($4)
	lwc1   $f5,    5*4($4)
	lwc1   $f6,    6*4($4)
	lwc1   $f7,    7*4($4)
	lwc1   $f8,    8*4($4)
	lwc1   $f9,    9*4($4)
	lwc1  $f10,   10*4($4)
	lwc1  $f11,   11*4($4)
	lwc1  $f12,   12*4($4)
	lwc1  $f13,   13*4($4)
	lwc1  $f14,   14*4($4)
	lwc1  $f15,   15*4($4)
	lwc1  $f16,   16*4($4)
	lwc1  $f17,   17*4($4)
	lwc1  $f18,   18*4($4)
	lwc1  $f19,   19*4($4)
	lwc1  $f20,   20*4($4)
	lwc1  $f21,   21*4($4)
	lwc1  $f22,   22*4($4)
	lwc1  $f23,   23*4($4)
	lwc1  $f24,   24*4($4)
	lwc1  $f25,   25*4($4)
	lwc1  $f26,   26*4($4)
	lwc1  $f27,   27*4($4)
	lwc1  $f28,   28*4($4)
	lwc1  $f29,   29*4($4)
	lwc1  $f30,   30*4($4)
	lwc1  $f31,   31*4($4)

  /*
   * Move the context pointer to the
   * fPR control array.
   */

  addi $4, $4, 32 * 4

  /*
   * Load FP control registers.
   */

  lw    $26,    ($4)
	ctc1  $26,    $0

  lw    $26, 1*4($4)
	ctc1  $26,    $25

  lw    $26, 2*4($4)
	ctc1  $26,    $26

  lw    $26, 3*4($4)
	ctc1  $26,    $28

  lw    $26, 4*4($4)
	ctc1  $26,    $31

  /*
   * Move the context pointer to the
   * GPR array.
   */

  addi $4, $4, 5 * 4

	/*
	 * Loading regular register
	 */

	lw     $1,       ($4)
	lw     $2,    1*4($4)
	lw     $3,    2*4($4)
	lw     $5,    4*4($4)
	lw     $6,    5*4($4)
	lw     $7,    6*4($4)
	lw     $8,    7*4($4)
	lw     $9,    8*4($4)
	lw    $10,    9*4($4)
	lw    $11,   10*4($4)
	lw    $12,   11*4($4)
	lw    $13,   12*4($4)
	lw    $14,   13*4($4)
	lw    $15,   14*4($4)
	lw    $16,   15*4($4)
	lw    $17,   16*4($4)
	lw    $18,   17*4($4)
	lw    $19,   18*4($4)
	lw    $20,   19*4($4)
	lw    $21,   20*4($4)
	lw    $22,   21*4($4)
	lw    $23,   22*4($4)
	lw    $24,   23*4($4)
	lw    $25,   24*4($4)
	lw    $28,   25*4($4)
	lw    $29,   26*4($4)
	lw    $30,   27*4($4)
	lw    $31,   28*4($4)

	/*
	 * loading special registers LO, HI, CR and SR
	 */

	lw    $26,   29*4($4)
	mtlo  $26

	lw    $27,   30*4($4)
	mthi  $27

	lw    $26,   31*4($4)
	mtc0  $26,   $13

	/*
	 * Computing and restoring status
	 */

	mfc0  $26,   $12
  li    $27,   0x0000FF00
  and   $26,   $26,  $27
	lw    $27,   32*4($4) 
  or    $27,   $27,   $26

  /*
   * Little trick here: reg 4 needs to be saved
   * before restoring the status (and traps...)
   */

	lw    $4,    3*4($4)
	mtc0  $27,   $12

	/*
	 * Branching to the new context
	 */

	jr    $31
	nop
	.set at
	.set reorder
.end cpu_context_load
