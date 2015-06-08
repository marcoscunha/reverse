#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libdebughelper.h>

#include "print_lock_descr.h"

int print_lock_descr(lock_descr_t *lock_tab, int nb_entry)
{
  int i=0;
  int j=0;
  printf("lock_descr:\n");
  if (nb_entry == 0) {
    printf("%s\n", "NO ENTRY");
  } else {
    for (i=0 ; i<nb_entry ; i++) {
      printf("[0x%08llx ; %s] : ", lock_tab[i].magic_lock_descr.pc, lock_tab[i].magic_lock_descr.name);
      if (lock_tab[i].lock_location == NULL) {
        printf("%s\n", "NO DWARF INFO");
      } else {
        int no_of_ops = lock_tab[i].lock_location->ld_cents;
        for (j = 0; j < no_of_ops; j++) {
          Dwarf_Loc * expr = &lock_tab[i].lock_location->ld_s[j];
          Dwarf_Small op;
          const char *op_str = NULL;

          op = expr->lr_atom;
          int res = dwarf_get_OP_name(op,&op_str);

          if(res == DW_DLV_OK){
            printf("%s ", op_str);
          } else {
            printf("%s ", "ERROR IN DWARF");
          }
        }
        printf("\n");
      }

      if (lock_tab[i].lock_frame == NULL) {
        printf("%s\n", "NO FRAME INFO");
      } else {
        int no_of_ops = lock_tab[i].lock_frame->ld_cents;
        for (j = 0; j < no_of_ops; j++) {
          Dwarf_Loc * expr = &lock_tab[i].lock_frame->ld_s[j];
          Dwarf_Small op;
          const char *op_str = NULL;

          op = expr->lr_atom;
          int res = dwarf_get_OP_name(op,&op_str);

          if(res == DW_DLV_OK){
            printf("%s ", op_str);
          } else {
            printf("%s ", "ERROR IN DWARF");
          }
        }
        printf("\n");
      }

	}
  }
  return 0;
}

