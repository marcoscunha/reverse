.section .text
	.align 4
	.global Int
	.extern PLATFORM_AICU_BASE

.ent Int
Int :   
	.set noat
	.set noreorder

  mfc0  $4, $13            /*  load CAUSE */
  mfc0  $5, $12            /*  load STATUS */
  andi  $4, $4,  0xFF00    /*  mask CAUSE to get the ITs only */
  and   $4, $4,  $5        /*  keep only the enabled ITs */
  srl   $4, $4,  8         /*  and put them in the LSBees */
  beqz  $4, _endit                                /* If none, then what are we doing here? */
  nop

  /*
   * Get the processor ID and the AICU base.
   */

  mfc0  $6, $0
	la    $26, PLATFORM_AICU_BASE
  lw    $26, ($26)
  sll   $6, 0x4
  add   $26, $26, $6
  addi  $26, $26, 0x100

  /*
   * Get the local interrupt ID.
   */

	lw    $4, 12($26)
  nop

 /*
  * Jump to the handler.
  */

	lw		$27, 8($26)
	nop
	jr    $27 /* not 'jal': handler will return to exception handler */
  nop

_endit:
	jr    $31 /*  go back to user*/
	nop
	.set at
	.set reorder
.end Int
