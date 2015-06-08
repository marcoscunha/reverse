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
#include <ctxt_stack.h>

#ifdef CONTEXT_DEBUG
#define DEBUG
#endif

#define DBG_HDR "context"
#include <debug.h>

/**
 * @brief
 *
 *
 * @param call
 * @param func
 * @return
 */
ctxt_stack_t *
ctxt_new(call_t *call, function_t *func){

	ctxt_stack_t *res = malloc(sizeof(ctxt_stack_t));

	res->lower = NULL;
	res->call  = call;
	res->func  = func;
	res->level = 0;
	res->flags = CTXT_FLAGS_NONE;

	return res;
}

/**
 * @brief Context exclusion
 *
 * Deallocation of context pointed by ctxt
 *
 * @param ctxt Pointer context to be deallocated
 */
void
ctxt_free(ctxt_stack_t *ctxt){

	free(ctxt);
	return;
}

/**
 * @brief Push a new context in stack
 *
 * Push a new context in stack of functions for especific context stack
 *
 * @param stack Stack of called function
 * @param ctxt Context to be pushed
 */
void
ctxt_push(ctxt_stack_t **stack, ctxt_stack_t *ctxt){

	if(ctxt == NULL){
		EMSG("Trying to insert an empty context !!\n");
		return;
	}

	ctxt->lower = *stack;
	if(*stack)
		ctxt->level = (*stack)->level + 1;
	*stack = ctxt;
	return;
}

/**
 * @brief Pop a context from context stack
 *
 *
 * @param stack Pointer to pointer context stack
 * @return
 *
 */

ctxt_stack_t *
ctxt_pop(ctxt_stack_t **stack){

	if(*stack == NULL){
		EMSG("Poping an empty stack\n");
		return NULL;
	}
	ctxt_stack_t *res = *stack;
	*stack = res->lower;

	res->lower = NULL;
	return res;
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
