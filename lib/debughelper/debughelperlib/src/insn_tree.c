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

#include <insn.h>
#include <insn_tree.h>
#include <insn_list.h>

#ifdef INSN_DEBUG
#define DEBUG
#endif

#define DBG_HDR "insn"
#include <debug.h>

ilist_elt_t  *
itree_search_filtered_l(itree_node_t *tree, unsigned long skey, unsigned long sfilter){

	if(tree == NULL)
		return NULL;
	if( (tree->key == skey) &&
	    ( IT_INSN(tree)->flags & sfilter ) ){
		return (ilist_elt_t *)tree->data;
	}

	if(tree->key < skey){
		ilist_elt_t *res = itree_search_filtered_l(tree->right, skey, sfilter);
		if(res)
			return res;

		if(IT_INSN(tree)->flags & sfilter)
			return (ilist_elt_t *)tree->data;
	}

	return itree_search_filtered_l(tree->left, skey, sfilter);

}


ilist_elt_t  *
itree_search_filtered_u(itree_node_t *tree __attribute__((unused)),
                        unsigned long skey __attribute__((unused)),
                        unsigned long sfilter __attribute__((unused))){

	/* Not yet implemented ... even useless */
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
