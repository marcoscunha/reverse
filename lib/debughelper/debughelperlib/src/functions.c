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
#include <functions.h>

#ifdef FUNCTION_DEBUG
#define DEBUG
#endif

#define DBG_HDR "function"
#include <debug.h>

function_t *
function_new(void){

	function_t *res;

	res = malloc(sizeof(function_t));
	res->next       = NULL;
	res->name       = NULL;
	res->entry_pc   = 0;
	res->lowpc      = 0;
	res->highpc     = 0;
	res->CU_idx     = 0;
	res->die_off    = 0;
	res->die_origin = 0;
	res->type       = FUNCTION_TYPE_NONE;
	res->abstr_func = NULL;
	res->syms       = NULL;
   res->nb_ranges  = 0;
   res->ranges     = NULL;
   res->declaration = 0;
   res->fb_locs = NULL;
   res->nb_fb_locs = 0;

	return res;
}

function_t *
function_copy(function_t *src){

	function_t *res;

	res = malloc(sizeof(function_t));
	res->next        = NULL;
	res->name        = src->name;
	res->type        = src->type;
	res->entry_pc    = src->entry_pc;
	res->lowpc       = src->lowpc;
	res->highpc      = src->highpc;
	res->CU_idx      = src->CU_idx;
	res->die_off     = src->die_off;
	res->die_origin  = src->die_origin;
	res->type        = src->type;
	res->abstr_func  = src->abstr_func;
	res->syms        = src->syms;
   res->declaration = src->declaration;
   res->fb_locs     = src->fb_locs;
   res->nb_fb_locs  = src->nb_fb_locs;
	return res;
}

void
function_dump(function_t *func){

	if(func->name) {
		IMSG("<%5lu> [Function] Name   : %s\n", func->die_off, func->name);
	} else {
		if(func->die_origin){
			IMSG("<%5lu> [Function] orig   : %lu\n", func->die_off, func->die_origin);
		}else{
			IMSG("<%5lu> [Function] Name   : {NO_NAME}\n", func->die_off);
		}
	}
	IMSG("        [Function] Type   : %d\n", func->type);
	IMSG("        [Function] Lowpc  : 0x%08x\n", func->lowpc);
	IMSG("        [Function] Highpc : 0x%08x\n", func->highpc);

	return;
}

void
function_free(function_t *func){

   slist_free(func->syms);
   func->syms = NULL;
	if(!func->abstr_func)
		free(func->name);
	free(func);

	return;
}

void
flist_walk(function_t *list, void (*hook)(void *hookarg, function_t *func), void *hookarg){

	if(list == NULL){
		return;
	}
	flist_walk(list->next, hook, hookarg);
	hook(hookarg, list);

}

void
flist_free(function_t *list){

   /* TODO: desallocating fb_locs will be complicated */
   
	if(list == NULL){
		return;
	}else{
		flist_free(list->next);
		function_free(list);
		return;
	}
}

void
flist_dump(function_t *list){
  
	if(list == NULL){
		return;
	}else{
		flist_dump(list->next);
		function_dump(list);
		return;
	}
}

void
flist_add_head(function_t **list, function_t *new){
  
	if(new == NULL){
		EMSG("Adding an empty entry !!\n");
		return;
	}
	new->next = *list;
	*list = new; 
	return;
}

function_t *
flist_lookup(function_t *list, unsigned long CU_idx, unsigned long die_off){

	if(list == NULL)
		return NULL;
	if( (list->CU_idx  == CU_idx ) &&
	    (list->die_off == die_off) )
		return list;
	return flist_lookup(list->next, CU_idx, die_off);
}

function_t *
flist_lookup_entry(function_t *list, unsigned long entry){

	if(list == NULL)
		return NULL;
	if(list->lowpc == entry)
		return list;
	return flist_lookup_entry(list->next, entry);
}

function_t *
flist_lookup_addr(function_t *list, addr_t addr){

	if(list == NULL)
		return NULL;

   if(IS_IN_FUNCTION_RANGE(addr, list))
		return list;
	return flist_lookup_addr(list->next, addr);
}

function_t *
flist_lookup_name(function_t *list, char *sym){

       if(list == NULL)
               return NULL;
       if(strcmp(list->name, sym) == 0)
               return list;
       return flist_lookup_name(list->next, sym);
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
