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

#ifndef _INSN_TREE_H
#define _INSN_TREE_H

#include <avl_tree.h>
#include <insn_list.h>

#define FORCE_INLINE __attribute__((always_inline))

typedef avl_tree_t itree_node_t;
/******************************
 * Equivalent structure       *
 ******************************
 * struct itree_node {        *
 *                            *
 *   itree_node_t  *right;    *
 *   itree_node_t  *left;     *
 *                            *
 *   unsigned long  key;      *
 *   unsigned long  depth;    *
 *   long           flags;    *
 *                            *
 *   ilist_elt_t   *data;     *
 * };                         *
 ******************************/

static inline itree_node_t *itree_new(unsigned long key, ilist_elt_t *data) FORCE_INLINE;
static inline void          itree_free_node(itree_node_t *node) FORCE_INLINE;
static inline void          itree_free(itree_node_t *tree) FORCE_INLINE;

static inline void          itree_ordered_dump(itree_node_t *tree) FORCE_INLINE;
static inline void          itree_dump(itree_node_t *tree) FORCE_INLINE;

static inline void          itree_insert(itree_node_t **tree, itree_node_t *node) FORCE_INLINE;
static inline itree_node_t *itree_search(itree_node_t *tree, unsigned long key) FORCE_INLINE;

ilist_elt_t  *itree_search_filtered_l(itree_node_t *tree, unsigned long skey, unsigned long sfilter);
ilist_elt_t  *itree_search_filtered_u(itree_node_t *tree, unsigned long skey, unsigned long sfilter);


itree_node_t *
itree_new(unsigned long key, ilist_elt_t *data){
	return (itree_node_t *)avl_new(key, (void *)data);
}

void
itree_free_node(itree_node_t *node){
	avl_free_node((avl_tree_t *)node);
}

void
itree_free(itree_node_t *node){
	avl_free((avl_tree_t *)node);
}

void
itree_insert(itree_node_t **tree, itree_node_t *node){
	avl_insert((avl_tree_t **)tree, (avl_tree_t *)node);
}

itree_node_t *
itree_search(itree_node_t *tree, unsigned long key){
	return (itree_node_t *)avl_search((avl_tree_t *)tree, key);
}

void
itree_dump(itree_node_t *tree){
	avl_dump((avl_tree_t *)tree, 0);

}

void
itree_ordered_dump(itree_node_t *tree){
	avl_walk((avl_tree_t *)tree, (void(*)(void *))ilist_dump_elt);
}


#define IT_INSN(a) (IL_INSN((ilist_elt_t *)((a)->data)))

#endif /* _INSN_TREE_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
