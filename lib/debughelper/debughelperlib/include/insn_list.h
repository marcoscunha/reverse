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

#ifndef _INSN_LIST_H
#define _INSN_LIST_H

#include <generic_olist.h>
#include <insn.h>

#define FORCE_INLINE __attribute__((always_inline))

typedef olist_elt_t ilist_elt_t;
/******************************
 * Equivalent structure       *
 ******************************
 * struct ilist_elt {         *
 *   ilist_elt_t *next;       *
 *   unsigned int key         *
 *   insn_t      *data;       *
 * };                         *
 ******************************/

static inline ilist_elt_t *ilist_new(insn_t *data) FORCE_INLINE;
void                       ilist_free(ilist_elt_t *elt);
static inline void         ilist_free_elt(ilist_elt_t *list) FORCE_INLINE;
static inline ilist_elt_t *ilist_add(ilist_elt_t *list, ilist_elt_t *elt) FORCE_INLINE;
static inline ilist_elt_t *ilist_search(ilist_elt_t *list, pc_t key) FORCE_INLINE;
void                       ilist_dump(ilist_elt_t *list);
void                       ilist_dump_elt(ilist_elt_t *elt);

ilist_elt_t *
ilist_new(insn_t *insn) {
	return (ilist_elt_t *)olist_new((unsigned int)insn->addr, (void *)insn);
}

void
ilist_free_elt(ilist_elt_t *list){
	return olist_free_elt((olist_elt_t *)list);
}

ilist_elt_t *
ilist_add(ilist_elt_t *list, ilist_elt_t *elt){
	return (ilist_elt_t *)olist_add((olist_elt_t *)list, (olist_elt_t *)elt);
}

ilist_elt_t *
ilist_search(ilist_elt_t *list, pc_t key){
	return (ilist_elt_t *)olist_search((olist_elt_t *)list, (unsigned int)key);
}

#define IL_INSN(a)  (((insn_t *)(a)->data))
#define IL_NEXT(a)  (((a)->next))
#define IL_ADDR(a)  (((insn_t *)(a)->data)->addr)

#endif /* _INSN_LIST_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
