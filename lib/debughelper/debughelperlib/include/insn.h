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

#ifndef _INSN_H
#define _INSN_H

#include <stdint.h>

typedef struct insn  insn_t;

#include <functions.h>
#include <calls.h>
#include <counters.h>
#include <dwarf_nfo.h>

#include <common_types.h>

#define INSN_FLAGS_NONE          0x00 /* no flags ... typical instruction */
#define INSN_FLAGS_UNUSED        0x01 /* dummy instruction                */
#define INSN_FLAGS_FUNC_ENTRY    0x02 /* function entry                   */
#define INSN_FLAGS_FUNC_END      0x04 /* function def end                 */
#define INSN_FLAGS_FUNC_CALL     0x08 /* function call                    */
#define INSN_FLAGS_JUMP          0x10 /* jump                             */
#define INSN_FLAGS_NEW_LINE      0x20 /* new source line                  */
#define INSN_FLAGS_FUNC_RANGE    0x40 /* func range                       */


struct insn{

	/* most important information */
	uint32_t       flags;
	pc_t           addr;

	pc_t           ret_addr;

	function_t    *func;
	line_nfo_t    *line;
	char          *fname;

	call_t        *calls;

	counters_t    *costs;

};

insn_t        *insn_new(pc_t);
void           insn_free(insn_t *);
void           insn_dump(insn_t *);

#endif /* _INSN_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
