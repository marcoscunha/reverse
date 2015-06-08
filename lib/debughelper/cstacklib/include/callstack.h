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

#ifndef _CALLSTACK_H
#define _CALLSTACK_H

typedef struct callstack      callstack_t;
typedef enum callstack_lookup callstack_lookup_t;
typedef enum callstack_state  callstack_state_t;

#include <common_types.h>
#include <libdebughelper.h>
#include <ctxt_stack.h>

#define INSN_FLAGS_NSEQ 0x00000001
#define INSN_FLAGS_EXC  0x00000002

enum callstack_lookup {
   CALLSTACK_NONE,
   CALLSTACK_CALL,
   CALLSTACK_JUMP,
   CALLSTACK_RET,
   CALLSTACK_CTXTSW,
   CALLSTACK_WAIT_CALL,
   CALLSTACK_ERROR,
};

enum callstack_state {
   CS_STATE_UNINITIALIZED,
   CS_STATE_INITIALIZED,
   CS_STATE_ERROR,
};

struct callstack{

   debug_helper_t    *dh;
	callstack_state_t  state;
   ilist_elt_t       *prev_insn;
   ilist_elt_t       *curr_insn;
   
   ctxt_stack_t      *stack;
   ctxt_stack_t      *normal_stack;
   ctxt_stack_t      *except_stack;

};

callstack_t          *callstack_new(char *fname);
void                  callstack_prepare(callstack_t *trace, debug_nfo_t *debug);
void                  callstack_free(callstack_t *trace);

callstack_lookup_t    callstack_lookup(callstack_t *trace, insn_desc_t *insn,
                                        call_t **call);
void                  callstack_pretty_print(callstack_t *trace);
void                  callstack_dump(callstack_t *trace, call_t **call);

#endif /* _CALLSTACK_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
