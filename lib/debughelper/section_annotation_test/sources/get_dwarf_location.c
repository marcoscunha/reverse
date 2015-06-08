#include <stdio.h>
#include <stdlib.h>
#include <functions.h>
#include <libdebughelper.h>

#include "get_dwarf_location.h"

void
print_location(Dwarf_Locdesc *loc){

	 int i = 0;
	 int no_of_ops = loc->ld_cents;

	 for (i = 0; i < no_of_ops; i++) {
		  Dwarf_Loc * expr = &loc->ld_s[i];
		  Dwarf_Small op;
		  const char *op_str = NULL;
			   
		  op = expr->lr_atom;
		  int res = dwarf_get_OP_name(op,&op_str);

		  if(res == DW_DLV_OK){
			   printf("OP: %s\n", op_str);
		  }

		  if (op >= DW_OP_lit0 && op <= DW_OP_reg31) {
			   /* Nothing to add. */
		  } else if (op >= DW_OP_breg0 && op <= DW_OP_breg31) {

		  }else {
			   switch (op) {
			   case DW_OP_addr:
			   case DW_OP_const1s:
			   case DW_OP_const2s:
			   case DW_OP_const4s:
			   case DW_OP_const8s:
			   case DW_OP_consts:
			   case DW_OP_skip:
			   case DW_OP_bra:
			   case DW_OP_fbreg:
#if 0
			   case DW_OP_GNU_const_index:
			   case DW_OP_GNU_addr_index:
			   case DW_OP_addrx: /* DWARF5: unsigned val */
			   case DW_OP_constx: /* DWARF5: unsigned val */
#endif
			   case DW_OP_const1u:
			   case DW_OP_const2u:
			   case DW_OP_const4u:
			   case DW_OP_const8u:
			   case DW_OP_constu:
			   case DW_OP_pick:
			   case DW_OP_plus_uconst:
			   case DW_OP_regx:
			   case DW_OP_piece:
			   case DW_OP_deref_size:
			   case DW_OP_xderef_size:
			   case DW_OP_bregx:
			   case DW_OP_call2:
			   case DW_OP_call4:
			   case DW_OP_call_ref:
			   case DW_OP_bit_piece:
			   case DW_OP_implicit_value:
			   case DW_OP_HP_unknown:
			   case DW_OP_HP_is_value:
			   case DW_OP_HP_fltconst4:
			   case DW_OP_HP_fltconst8:
			   case DW_OP_HP_mod_range:
			   case DW_OP_HP_unmod_range:
			   case DW_OP_HP_tls:
			   case DW_OP_INTEL_bit_piece:
			   case DW_OP_stack_value:  /* DWARF4 */
			   case DW_OP_GNU_uninit:  /* DW_OP_APPLE_uninit */
			   case DW_OP_GNU_encoded_addr:
			   case DW_OP_GNU_implicit_pointer:
			   case DW_OP_GNU_entry_value:
#if 0
			   case DW_OP_GNU_const_type:
			   case DW_OP_GNU_regval_type:
			   case DW_OP_GNU_deref_type:
			   case DW_OP_GNU_convert:
			   case DW_OP_GNU_reinterpret:
			   case DW_OP_GNU_parameter_ref:
#endif
			   default:
					break;
			   }
		  }
	 }
}




int get_dwarf_location(debug_helper_t *dh, lock_descr_t *lock_tab, int nb_entry)
{
  Dwarf_Locdesc *result = NULL;
  Dwarf_Locdesc *frame = NULL;
  int i=0;

  for (i=0 ; i<nb_entry ; i++) {
    get_loc_ret_t ret = dh_get_loc(dh, lock_tab[i].magic_lock_descr.pc,
								   lock_tab[i].magic_lock_descr.name, &result,
								   &frame);
    if(ret != DH_LOC_FOUND){
		 printf("Got no loc: %d\n", ret);
		 lock_tab[i].lock_location = NULL;
	}else
		 lock_tab[i].lock_location = result;
		 lock_tab[i].lock_frame = frame;
  }

  return 0;
}
