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
#include <avl_tree.h>

#ifdef AVL_DEBUG
#define DEBUG
#endif

#define DBG_HDR "avl_tree"
#include "debug.h"

#define MAX(a, b) ((a)>(b)?(a):(b))


/******************************************************************
 *
 *  Internal functions
 *
 ******************************************************************/


/******************************************************************
 *                         Right Rotation                         *
 *                                                                *
 *                -----                       -----               *
 *           Root | 1 |                       | 2 |               *
 *                -----                       -----               *
 *              /       \                   /       \             *
 *        -----         /\                 /\        -----        *
 *  Pivot | 2 |        /  \      =>       /  \       | 1 |        *
 *        -----       / R1 \             / L2 \      -----        *
 *      /       \    --------           --------   /       \      *
 *     /\       /\                                /\       /\     *
 *    /  \     /  \                              /  \     /  \    *
 *   / L2 \   / R2 \                            / R2 \   / R1 \   *
 *  -------- --------                          -------- --------  *
 *                                                                *
 ******************************************************************/

void
avl_right_rotate(avl_tree_t **root){

	avl_tree_t *node1 = (*root);
	avl_tree_t *node2 = (*root)->left;
	int r1_d = 0, l2_d = 0, r2_d = 0;
	int n1_d = 0, n2_d = 0;

	if(node2->left){
		l2_d = node2->left->depth;
	}
	if(node2->right){
		r2_d = node2->right->depth;
	}
	if(node1->right){
		r1_d = node1->right->depth;
	}

	node1->left = node2->right;
	node2->right = node1;

	/* Update depths and flags */
	n1_d = MAX(r2_d, r1_d) + 1;
	n2_d = MAX(l2_d, n1_d) + 1;

	node1->depth = n1_d;
	node1->flags = r1_d - r2_d;
	node2->depth = n2_d;
	node2->flags = n1_d - l2_d;

	*root = node2;
	return;
}

/******************************************************************
 *                         Left Rotation                          *
 *                                                                *
 *            -----                               -----           *
 *            | 1 | Root                          | 2 |           *
 *            -----                               -----           *
 *          /       \                           /       \         *
 *         /\        -----                -----         /\        *
 *        /  \       | 2 | Pivot ==>      | 1 |        /  \       *
 *       / L1 \      -----                -----       / R2 \      *
 *      --------   /       \            /       \    --------     *
 *                /\       /\          /\       /\                *
 *               /  \     /  \        /  \     /  \               *
 *              / L2 \   / R2 \      / L1 \   / L2 \              *
 *             -------- --------    -------- --------             *
 *                                                                *
 ******************************************************************/

void
avl_left_rotate(avl_tree_t **root){

	avl_tree_t *node1 = (*root);
	avl_tree_t *node2 = (*root)->right;
	int l1_d = 0, l2_d = 0, r2_d = 0;
	int n1_d = 0, n2_d = 0;

	if(node2->left){
		l2_d = node2->left->depth;
	}
	if(node2->right){
		r2_d = node2->right->depth;
	}
	if(node1->left){
		l1_d = node1->left->depth;
	}

	node1->right = node2->left;
	node2->left = node1;

	/* Update depths and flags */
	n1_d = MAX(l1_d, l2_d) + 1;
	n2_d = MAX(n1_d, r2_d) + 1;

	node1->depth = n1_d;
	node1->flags = l2_d - l1_d;
	node2->depth = n2_d;
	node2->flags = r2_d - n1_d;


	*root = node2;
	return;
}

void
avl_balance(avl_tree_t **root){

	HMSG("balance() ... \n");

	switch( (*root)->flags ){
    
	case AVL_FLAGS_LEFT_UNBAL:
    {
		avl_tree_t *left_root = (*root)->left;
		switch(left_root->flags){

		case AVL_FLAGS_BALANCED:
			IMSG("Balanced sub-tree\n");
			break;
		case AVL_FLAGS_LEFT_HEAVY:
			/* Nothing to do ... */
			HMSG("LEFT LEFT\n");
			break;
		case AVL_FLAGS_RIGHT_HEAVY:
			HMSG("LEFT RIGHT\n");
			avl_left_rotate( &((*root)->left));
			break;

		case AVL_FLAGS_LEFT_UNBAL:
		case AVL_FLAGS_RIGHT_UNBAL:
			EMSG("Unbalanced sub-tree !!!\n");
			break;
		default:
			EMSG("Inconsistent Flags\n");
		}

		avl_right_rotate(root);

    }
    break;
	case AVL_FLAGS_RIGHT_UNBAL:
    {
		avl_tree_t *right_root = (*root)->right;
		switch(right_root->flags){

		case AVL_FLAGS_BALANCED:
			IMSG("Balanced sub-tree\n");
			break;
		case AVL_FLAGS_LEFT_HEAVY:
			HMSG("RIGHT LEFT\n");
			avl_right_rotate( &((*root)->right));
			break;
		case AVL_FLAGS_RIGHT_HEAVY:
			/* Nothing to do ... */
			HMSG("RIGHT RIGHT\n");
			break;

		case AVL_FLAGS_LEFT_UNBAL:
		case AVL_FLAGS_RIGHT_UNBAL:
			EMSG("Unbalanced sub-tree !!!\n");
			break;
		default:
			EMSG("Inconsistent Flags\n");
		}

		avl_left_rotate(root);

    }
    break;
	case AVL_FLAGS_BALANCED:
	case AVL_FLAGS_LEFT_HEAVY:
	case AVL_FLAGS_RIGHT_HEAVY:
		EMSG("useless call of avl_balance()\n");
		break;
	default:
		EMSG("Inconsistent Flags\n");
	}

}


/******************************************************************
 *
 *  Public functions
 *
 ******************************************************************/

avl_tree_t *
avl_new(unsigned long key, void *data){

	avl_tree_t *res = malloc(sizeof(avl_tree_t));
	res->right = NULL;
	res->left  = NULL;

	res->depth = 0;
	res->key   = key;
	res->flags = AVL_FLAGS_BALANCED;
	res->data  = data;

	return res;
}

void
avl_free_node(avl_tree_t *node){
	free(node);
}

void
avl_free(avl_tree_t *root){

	if(root == NULL){
		return;
	}
	avl_free(root->left);
	avl_free(root->right);
	avl_free_node(root);

}

avl_tree_t *
avl_search(avl_tree_t *root, unsigned long search){

	if(root == NULL){
		return NULL;
	}
   
	if( root->key > search ) {

		return avl_search(root->left, search);
 
	}else{
		if( root->key < search ){
			avl_tree_t *res = avl_search(root->right, search);
			if( !res ){ /* if no closer solution */
				return root;
			}else{
				return res;
			}
		}else{ /* root->key == search */
			return root;
		}
	}

	EMSG("Should not being here\n");
	return NULL;

}

void
avl_insert(avl_tree_t **root, avl_tree_t *node){

	int left_d = 0, right_d = 0;

	/* Break condition ... */
	if( (*root) == NULL ){
		(*root)     = node;
		node->depth = 1;
		return;
	}

	if( (*root)->key > node->key ){

		avl_insert( &((*root)->left), node);
    
	} else {
		if( (*root)->key < node->key ){

			avl_insert( &((*root)->right), node);

		} else { /* (*root)->key == node->key */

			EMSG("Trying to duplicate entry ...\n");
			return;

		}
	}

	if( (*root)->left ){
		left_d = (*root)->left->depth;
	}
	if( (*root)->right ){
		right_d = (*root)->right->depth;
	}

	(*root)->depth = MAX(left_d, right_d) + 1;
	(*root)->flags = right_d - left_d;

	if( ((*root)->flags == AVL_FLAGS_LEFT_UNBAL) ||
	    ((*root)->flags == AVL_FLAGS_RIGHT_UNBAL) ){
		avl_balance(root);
	}
	return;
}

void
avl_walk(avl_tree_t *root, void(*hook)(void *)){

	if(root){
		avl_walk(root->left, hook);
		hook(root->data);
		avl_walk(root->right, hook);
	}
}

void
avl_dump(avl_tree_t *root, long lvl){

	int i = 0;
	char spacer[1024];
	char *flag;

	for( i = 0; (i < (lvl*8)) && (i < 128); i++){
		spacer[i] = ' ';
	}
	spacer[i] = '\0';

	if( root == NULL ){
		printf("%s[NULL]\n", spacer);
	}else{    

		switch(root->flags){
		case AVL_FLAGS_BALANCED:
			flag="BA";
			break;
		case AVL_FLAGS_LEFT_HEAVY:
			flag="LH";
			break;
		case AVL_FLAGS_RIGHT_HEAVY:
			flag="RH";
			break;
		case AVL_FLAGS_LEFT_UNBAL:
			flag="LU";
			break;
		case AVL_FLAGS_RIGHT_UNBAL:
			flag="RU";
			break;
		default:
			EMSG("Bad Flag\n");
			flag="NO";
		}


		printf("%s[%4lu] ------- depth : %lu flag : %s\n",
		       spacer, root->key, root->depth, flag);
		printf("%s       -LEFT--\n", spacer);
		avl_dump(root->left, lvl+1);
		printf("%s       -RIGHT-\n", spacer);
		avl_dump(root->right, lvl+1);
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
