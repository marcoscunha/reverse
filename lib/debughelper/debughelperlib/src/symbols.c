/*
 * Copyright (c) ENS Lyon/LIP      2004-2007
 *               INSA Lyon/CITI    2004
 *               INRIA             2004
 *               INP Grenoble/Tima 2008
 *            Nicolas Fournel   <nicolas.fournel@imag.fr>
 *            Antoine Fraboulet <antoine.fraboulet@insa-lyon.fr>
 * 
 * This software is a computer program whose purpose is to process an 
 * etrace file in order to give divers informations on energy consumption.
 * 
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */

#include <string.h>
#include <stdlib.h>
#include <symbols.h>

#ifdef SYMBOL_DEBUG
#define DEBUG
#endif

#define DBG_HDR "symbol"
#include <debug.h>

symbol_t *
symbol_new(void){

	symbol_t *res;

	res = malloc(sizeof(symbol_t));
	res->next       = NULL;
	res->name       = NULL;
	res->CU_idx     = 0;
	res->die_off    = 0;
	res->die_origin = 0;
   res->locs       = NULL;
	return res;
}

void
symbol_dump(symbol_t *sym){

	if(sym->name) {
		IMSG("<0x%08lx> [Symbol] Name   : %s\n", sym->die_off, sym->name);
	} else {
		if(sym->die_origin){
			IMSG("<0x%08lx> [Symbol] orig   : %lu\n", sym->die_off, sym->die_origin);
		}else{
			IMSG("<0x%08lx> [Symbol] Name   : {NO_NAME}\n", sym->die_off);
		}
	}
	return;
}

void
symbol_free(symbol_t *sym){

   int i = 0;
   if(sym != NULL){
      for(i = 0; i< sym->nb_locs; i++){
         dwarf_dealloc(sym->dw->dbg, sym->locs[i]->ld_s, DW_DLA_LOC_BLOCK);
         dwarf_dealloc(sym->dw->dbg, sym->locs[i], DW_DLA_LOCDESC);
      }
      free(sym->locs);
      free(sym->name);
   }
	free(sym);
	return;
}

void
slist_walk(symbol_t *list, void (*hook)(void *hookarg, symbol_t *sym), void *hookarg){

	if(list == NULL){
		return;
	}
	slist_walk(list->next, hook, hookarg);
	hook(hookarg, list);

}

void
slist_free(symbol_t *list){

	if(list == NULL){
		return;
	}else{
		slist_free(list->next);
		symbol_free(list);
		return;
	}
}

void
slist_dump(symbol_t *list){
  
	if(list == NULL){
		return;
	}else{
		slist_dump(list->next);
		symbol_dump(list);
		return;
	}
}

void
slist_add_head(symbol_t **list, symbol_t *new){
  
	if(new == NULL){
		EMSG("Adding an empty entry !!\n");
		return;
	}
	new->next = *list;
	*list = new; 
	return;
}

symbol_t *
slist_lookup(symbol_t *list, unsigned long CU_idx, unsigned long die_off){

	if(list == NULL)
		return NULL;
	if( (list->CU_idx  == CU_idx ) &&
	    (list->die_off == die_off) )
		return list;
	return slist_lookup(list->next, CU_idx, die_off);
}

symbol_t *
slist_lookup_name(symbol_t *list, char *sym){

       if(list == NULL)
               return NULL;
       if(list->name && (strcmp(list->name, sym) == 0))
               return list;
       return slist_lookup_name(list->next, sym);
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
