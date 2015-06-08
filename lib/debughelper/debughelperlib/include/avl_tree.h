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

#ifndef _AVL_TREE_H
#define _AVL_TREE_H

#include "common_types.h"

typedef struct avl_tree avl_tree_t;

#define AVL_FLAGS_BALANCED       0
#define AVL_FLAGS_LEFT_HEAVY    -1
#define AVL_FLAGS_RIGHT_HEAVY    1
#define AVL_FLAGS_LEFT_UNBAL    -2
#define AVL_FLAGS_RIGHT_UNBAL    2

struct avl_tree {

	avl_tree_t    *right;
	avl_tree_t    *left;

	unsigned long  key;
	unsigned long  depth;
	long           flags;

	/* opaque data */
	void          *data;
};


avl_tree_t *avl_new(unsigned long key, void *data);
void        avl_free_node(avl_tree_t *node);
void        avl_free(avl_tree_t *tree);

void        avl_insert(avl_tree_t **root, avl_tree_t *node);
avl_tree_t *avl_search(avl_tree_t *root, unsigned long search);

void        avl_walk(avl_tree_t *root, void(*hook)(void *));
void        avl_dump(avl_tree_t *root, long lvl);

#endif /* _AVL_TREE_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
