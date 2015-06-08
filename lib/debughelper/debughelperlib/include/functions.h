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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

typedef struct function function_t;

#include <common_types.h>
#include <symbols.h>


#define FUNCTION_TYPE_NONE   0x00
#define FUNCTION_TYPE_FUNC   0x01
#define FUNCTION_TYPE_INLINE 0x02
#define FUNCTION_TYPE_SCOPE  0x04

typedef addr_t range_t[2];

struct function{

	function_t    *next;

	uint8_t        type;

   addr_t         entry_pc;
   int            nb_ranges;
   range_t       *ranges;
   
   addr_t         lowpc;
	addr_t         highpc;
	char          *name;

	function_t    *abstr_func;

	unsigned long  CU_idx;
	unsigned long  die_off;
	unsigned long  die_origin;
	unsigned long  decl_file;
	unsigned long  decl_line;
   int            declaration;
	symbol_t      *syms;

   Dwarf_Locdesc **fb_locs;
   int             nb_fb_locs;

};

function_t *function_new(void);
function_t *function_copy(function_t *src);
void        function_dump(function_t *func);
void        function_free(function_t *func);

void        flist_walk(function_t *list, void (*hook)(void *hookarg, function_t *func), void *hookarg);

void        flist_free(function_t *list);
void        flist_dump(function_t *list);
void        flist_add_head(function_t **list, function_t *new);
function_t *flist_lookup(function_t *list, unsigned long CU_idx, unsigned long die_off);
function_t *flist_lookup_entry(function_t *list, unsigned long entry);
function_t *flist_lookup_name(function_t *list, char *sym);
function_t *flist_lookup_addr(function_t *list, addr_t addr);

#define IS_IN_FUNCTION_RANGE(addr, func)				\
	( (addr >= func->lowpc) && (addr <= func->highpc) )


#endif /* _FUNCTIONS_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
