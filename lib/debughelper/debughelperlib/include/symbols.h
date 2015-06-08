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

#ifndef _SYMBOLS_H
#define _SYMBOLS_H

typedef struct symbol symbol_t;

#include <common_types.h>
#include <dwarf_nfo.h>

struct symbol{
	symbol_t      *next;

   dwarf_nfo_t   *dw;
	char          *name;
	unsigned long  CU_idx;
	unsigned long  die_off;
	unsigned long  die_origin;
	symbol_t      *abstr_sym;

   Dwarf_Locdesc **locs;
   int             nb_locs;
};

symbol_t *symbol_new(void);
void      symbol_dump(symbol_t *sym);
void      symbol_free(symbol_t *sym);

void      slist_walk(symbol_t *list, void (*hook)(void *hookarg, symbol_t *sym), void *hookarg);

void      slist_free(symbol_t *list);
void      slist_dump(symbol_t *list);
void      slist_add_head(symbol_t **list, symbol_t *new);
symbol_t *slist_lookup(symbol_t *list, unsigned long CU_idx, unsigned long die_off);
symbol_t *slist_lookup_name(symbol_t *list, char *sym);

#define IS_IN_LOCATION_RANGE(addr, loc)				\
	( (addr >= (loc)->ld_lopc) && (addr <= (loc)->ld_hipc) )

#endif /* _SYMBOLS_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
