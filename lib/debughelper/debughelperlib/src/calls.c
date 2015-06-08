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

#include <stdlib.h>
#include <calls.h>
#include <insn.h>

#ifdef CALL_DEBUG
#define DEBUG
#endif

#define DBG_HDR "call"
#include <debug.h>

/**
 * @brief Call initialization
 *
 * Call initialization
 *
 * @param caller Call instruction
 * @param callee Target to call
 * @return Pointer to a call
 */
call_t *
call_new(insn_t *caller, insn_t *callee){

	call_t *res = malloc(sizeof(call_t));

	res->next   = NULL;
	res->callee = callee;
	res->caller = caller;
	res->costs  = counters_new();

	return res;
}

/**
 *
 * @param call
 */
void
call_free(call_t *call){
  
	counters_free(call->costs);
	free(call);
	return;
}

/**
 *
 * @param list
 * @param new
 */
void
clist_add_head(call_t **list, call_t *new){

	if(new == NULL){
		EMSG("Inserting a Null call\n");
		return;
	}else{
		new->next = *list;
		*list     = new;
		return;
	}
}

/**
 *
 * @param list
 */
void
clist_free(call_t *list){

	if(list == NULL){
		return;
	}else{
		clist_free(list->next);
		call_free(list);
		return;
	}
}

/**
 *
 * @param list
 * @param caddr
 * @return
 */
call_t *
clist_search(call_t *list, unsigned int caddr){

   call_t *p = list;

	if(list == NULL)
		return NULL;
   
   while(p != NULL) {
      if(p->callee->addr == caddr)
         return p;

      p = p->next;
	}

   return NULL;
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
