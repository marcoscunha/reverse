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

#ifndef _DWARF_NFO_H
#define _DWARF_NFO_H

#include <unistd.h>
#include <libdwarf.h>
#include <dwarf.h>
#include <libelf.h>

typedef struct dwarf_nfo dwarf_nfo_t;

#include <common_types.h>
#include <functions.h>
#include <symbols.h>
#include <src_lines.h>


struct dwarf_nfo{
	/* Executable, Elf and Dwarf handlers */
	Elf          *elf;
	Dwarf_Debug   dbg;

   function_t   *curr_func;
   Dwarf_Addr    curr_CU_baseaddress;
   Dwarf_Addr    curr_CU_topaddress;
};

dwarf_nfo_t *dwarf_nfo_init(Elf *elf);
void         dwarf_nfo_closure(dwarf_nfo_t *dw);

void         dwarf_nfo_process_src(dwarf_nfo_t *dw, src_tab_t **lines);
void         dwarf_nfo_process_funcs(dwarf_nfo_t *dw, function_t **flist);


#define DBG_NFO_LOC_FOUND 0
#define DBG_NFO_NO_FUNC 1
#define DBG_NFO_NO_SYM 2
#define DBG_NFO_NO_LOC 3

int          dwarf_nfo_get_loc(dwarf_nfo_t *dw, function_t *func, uint32_t addr,
                               char *sym, Dwarf_Locdesc **loc, Dwarf_Locdesc **frame_base);

#endif /* _DWARF_NFO_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
