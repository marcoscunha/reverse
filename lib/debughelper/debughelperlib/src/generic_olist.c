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
#include <generic_olist.h>

#ifdef OLIST_DEBUG
#define DEBUG
#endif

#define DBG_HDR "olist"
#include <debug.h>

olist_elt_t *
olist_new(unsigned int key, void *data) {

	olist_elt_t *res = (olist_elt_t *)malloc(sizeof(olist_elt_t));
	res->next = NULL;
	res->key  = key;
	res->data = data;

	return res;
}

void
olist_free_elt(olist_elt_t *list){

	free(list);
}


olist_elt_t *
olist_add(olist_elt_t *list, olist_elt_t *new){

	if( (list == NULL) ||
	    (list->key > new->key) ){
		new->next = list;
		return new;
	}else{
		list->next = olist_add(list->next, new);
		return list;
	}

}

olist_elt_t *
olist_search(olist_elt_t *list, unsigned int key){

	if( (list == NULL) ||
	    (list->key == key) )
		return list;
	else
		return olist_search(list->next, key);

}

void
olist_for_each(olist_elt_t *list, void (*hook)(void *elt)){

	if(list==NULL){
		return;
	}else{
		hook(list->data);
		olist_for_each(list->next, hook);
	}

}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
