.section .text
	.align 4
	.global cpu_context_save

.ent cpu_context_save
cpu_context_save:
  .set noat
  .set noreorder

  /*
   * Saving FP registers.
   */

	swc1   $f0,       ($4)
	swc1   $f1,    1*4($4)
	swc1   $f2,    2*4($4)
	swc1   $f3,    3*4($4)
	swc1   $f4,    4*4($4)
	swc1   $f5,    5*4($4)
	swc1   $f6,    6*4($4)
	swc1   $f7,    7*4($4)
	swc1   $f8,    8*4($4)
	swc1   $f9,    9*4($4)
	swc1  $f10,   10*4($4)
	swc1  $f11,   11*4($4)
	swc1  $f12,   12*4($4)
	swc1  $f13,   13*4($4)
	swc1  $f14,   14*4($4)
	swc1  $f15,   15*4($4)
	swc1  $f16,   16*4($4)
	swc1  $f17,   17*4($4)
	swc1  $f18,   18*4($4)
	swc1  $f19,   19*4($4)
	swc1  $f20,   20*4($4)
	swc1  $f21,   21*4($4)
	swc1  $f22,   22*4($4)
	swc1  $f23,   23*4($4)
	swc1  $f24,   24*4($4)
	swc1  $f25,   25*4($4)
	swc1  $f26,   26*4($4)
	swc1  $f27,   27*4($4)
	swc1  $f28,   28*4($4)
	swc1  $f29,   29*4($4)
	swc1  $f30,   30*4($4)
	swc1  $f31,   31*4($4)

  /*
   * Move the context pointer to the
   * fPR control array.
   */

  addi $4, $4, 32 * 4


  /*
   * Save FP control registers.
   */

	cfc1  $26,    $0
  sw    $26,    ($4)

	cfc1  $26,    $25
  sw    $26, 1*4($4)

	cfc1  $26,    $26
  sw    $26, 2*4($4)

	cfc1  $26,    $28
  sw    $26, 3*4($4)

	cfc1  $26,    $31
  sw    $26, 4*4($4)

  /*
   * Move the context pointer to the
   * GPR array.
   */

  addi $4, $4, 5 * 4

	/*
	 * Saving regular registers
	 */

	sw     $1,       ($4)
	sw     $2,    1*4($4)
	sw     $3,    2*4($4)
	sw     $4,    3*4($4)
	sw     $5,    4*4($4)
	sw     $6,    5*4($4)
	sw     $7,    6*4($4)
	sw     $8,    7*4($4)
	sw     $9,    8*4($4)
	sw    $10,    9*4($4)
	sw    $11,   10*4($4)
	sw    $12,   11*4($4)
	sw    $13,   12*4($4)
	sw    $14,   13*4($4)
	sw    $15,   14*4($4)
	sw    $16,   15*4($4)
	sw    $17,   16*4($4)
	sw    $18,   17*4($4)
	sw    $19,   18*4($4)
	sw    $20,   19*4($4)
	sw    $21,   20*4($4)
	sw    $22,   21*4($4)
	sw    $23,   22*4($4)
	sw    $24,   23*4($4)
	sw    $25,   24*4($4)
	sw    $28,   25*4($4)
	sw    $29,   26*4($4)
	sw    $30,   27*4($4)
	sw    $5,   28*4($4)

	/*
	 * Saving special registers LO, HI, CR and SR
	 */

	mflo  $26
	sw    $26,   29*4($4)

	mfhi  $27
	sw    $27,   30*4($4)

	mfc0  $26,   $13
	sw    $26,   31*4($4)

	mfc0  $27,   $12
  li    $26,   0xFFFF00FF
  and   $27,   $27,  $26
	sw    $27,   32*4($4)

	/*
	 * Returning from the call
	 */

	jr    $31
	nop
  .set at
  .set reorder
.end cpu_context_save
