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

#include <libdebughelper.h>
#include <callstack.h>

#ifdef TRACE_DEBUG
#define DEBUG
#endif

#define DBG_HDR "callstack"
#include <debug.h>

callstack_t *
callstack_new(char *fname){

   callstack_t *res    = calloc(1, sizeof(callstack_t));
   call_t      *bottom = NULL;

   res->prev_insn = NULL;
   res->curr_insn = NULL;

   res->stack       = NULL;

   bottom            = call_new(NULL, NULL);
   res->normal_stack = ctxt_new(bottom, NULL);
   res->except_stack = NULL;
   res->stack        = res->normal_stack;

   res->state        = CS_STATE_UNINITIALIZED;

   res->dh           = dh_init(fname);
   
   return res;
}

void
callstack_free(callstack_t *trace){

   dh_free(trace->dh);
	free(trace);

	return;
}

void
callstack_dump(callstack_t *cstack, call_t **call){
   call_t *ret_call = NULL;
   call_t *pcall = NULL;
   /*
    * Flush the stack: to be moved in the stop operation.
    */
	while(cstack->stack){
      ctxt_stack_t *poped_ctxt = NULL;
      call_t       *found_call = NULL;

		poped_ctxt = ctxt_pop(&cstack->stack);
		pcall   = poped_ctxt->call;
		ctxt_free(poped_ctxt);

		if(!pcall->caller){
			HMSG("Got the final context\n");
			/* Nothing to update then trash */
      }else{

         DMSG("<----> (0xXXXXXXXX to 0xXXXXXXXX) [level %2d] < RET   %s  \n",
              cstack->stack->level,
              pcall->callee->func?pcall->callee->func->name:"NULL");

         found_call = clist_search(ret_call, pcall->callee->addr);
         
         pcall->func = pcall->callee->func;
         pcall->next = NULL;
 
         if(found_call == NULL){
            clist_add_head(&ret_call, pcall);
            pcall->costs->rep_counter++;
         }else{
            /* Update existing call costs */
            counters_sum(found_call->costs, ret_call->costs);
            found_call->costs->rep_counter++;
            call_free(pcall);
         }            
      }
   }
   *call = ret_call;
   return;
}

/**
 * @brief ...
 *
 * ...
 *
 * @param cs   callstack analysed
 * @param insn instructions definition of event analysed
 * @param next instructions definition of jump address
 * @param call ?
 * @return ?
 */
callstack_lookup_t
callstack_lookup(callstack_t *cs, insn_desc_t *insn, call_t **call){

   ilist_elt_t  *prev_insn = cs->prev_insn;
   ilist_elt_t  *curr_insn = NULL;

   uint32_t      addr   = insn->pc;

   *call = NULL;


   switch(cs->state){
     
      /************************************************************************
       * Initial State.                                                       *
       ************************************************************************/
   case CS_STATE_UNINITIALIZED: 
   {
      call_t       *new_call = NULL;
      call_t       *ret_call = NULL;
      ctxt_stack_t *ctxt     = NULL;
     
      curr_insn = dh_search(cs->dh, addr);

      if(!curr_insn){ 
         EMSG("Initial instruction not in a function: Folding deactivated\n");
         cs->state = CS_STATE_ERROR;
         return CALLSTACK_WAIT_CALL;
      }
      new_call = call_new(NULL, IL_INSN(curr_insn));
      ret_call = new_call;
      ctxt     = ctxt_new(new_call, IL_INSN(curr_insn)->func);
     
      ctxt_push(&cs->stack, ctxt);
     
      cs->prev_insn = curr_insn;  
                                
      ret_call->func = IL_INSN(curr_insn)->func;
      *call = ret_call;

      cs->state = CS_STATE_INITIALIZED;

      if( ret_call->func == NULL) {
         EMSG("No function name for initial instruction: Folding deactivated\n");
         cs->state = CS_STATE_ERROR;
         return CALLSTACK_ERROR;
      }

      printf("__entry_function: %s\n", ctxt->func->name);

      DMSG("<----> (0x%08x to 0x%08x) [level %2d] > CALL  %s\n",
           IL_INSN(curr_insn)->addr, addr,
           cs->stack->level,
           ctxt->func?ctxt->func->name:"NULL");
      return CALLSTACK_CALL; 
   }
   break;

   /***************************************************************************
    * Standard State.                                                         *
    ***************************************************************************/
   case CS_STATE_INITIALIZED:
   {
      ctxt_stack_t *pstack = cs->stack;
      int           depth  = 0;

      if(!(insn->flags & INSN_FLAGS_NSEQ)){
         HMSG("Shouldn't be called on a sequential instruction\n");
         return CALLSTACK_NONE;
      }
     
      curr_insn = dh_search(cs->dh, addr);

      if((IL_INSN(curr_insn)->addr == addr)                    &&
         (IL_INSN(curr_insn)->flags & INSN_FLAGS_FUNC_ENTRY)){
         /*
          * Push in the context_stack 
          */
         call_t       *ret_call = NULL;
         call_t       *new_call = NULL;
         ctxt_stack_t *ctxt     = NULL;

         /* new call context */
         new_call = call_new(IL_INSN(prev_insn), IL_INSN(curr_insn));
         ret_call = new_call;
         ctxt     = ctxt_new(new_call, IL_INSN(curr_insn)->func);
         ctxt_push(&cs->stack, ctxt);

         printf("__entry_function: %s\n", ctxt->func->name);

         DMSG("<----> (0x%08x to 0x%08x) [level %2d] > CALL  %s\n",
              IL_INSN(prev_insn)->addr, addr,
              cs->stack->level,
              ctxt->func?ctxt->func->name:"NULL");

         if(cs->stack->call->caller == NULL){
            EMSG("No context pushed ... \n");
         }

         ret_call->func = IL_INSN(curr_insn)->func;

         cs->prev_insn = curr_insn;

         *call = ret_call;

         if( ret_call->func == NULL) {
            EMSG("No function name for initial instruction: Folding deactivated \n");
            cs->state = CS_STATE_ERROR;
            return CALLSTACK_ERROR;
         }

         return CALLSTACK_CALL;

      }

      /*
       * First we will check if we are in the same scope.
       */
      /* ==================================================================== *
       * JUMPS                                                                *
       * ==================================================================== */
      if(cs->stack && cs->stack->func &&
         IS_IN_FUNCTION_RANGE(addr, cs->stack->func)){

         DMSG("<----> (0x%08x to 0x%08x) [level %2d] | JUMP\n",
              IL_INSN(prev_insn)->addr,
              IL_INSN(curr_insn)->addr,
              cs->stack->level);

         cs->prev_insn = curr_insn;

         *call = NULL;
         return CALLSTACK_JUMP;
      }
      /* ==================================================================== *
       * JUMPS (end)                                                          *
       * ==================================================================== */


      /*
       * Look in the current callstack to find returns
       */
      /* ==================================================================== *
       * RETURNS                                                              *
       * ==================================================================== */
      while(pstack){
         if(pstack->func                             &&
            IS_IN_FUNCTION_RANGE(addr, pstack->func) ){
            /* We found a matching context */
            break;
         }
         depth++;
         pstack = pstack->lower;
      }

      /* we found a corresponding context */
      if(pstack){
         call_t       *ret_call   = NULL;

         /* We have context to pop */
         while(depth--){
            if(!cs->stack->call->caller){
               EMSG("Poping the native context ... impossible!!!\n");
               cs->state = CS_STATE_ERROR;
               return CALLSTACK_ERROR;
            }
            /* Return to last function context */
            call_t       *found_call = NULL;
            call_t       *pcall   = NULL;
            ctxt_stack_t *poped_ctxt = NULL;

            poped_ctxt = ctxt_pop(&cs->stack);
            pcall   = poped_ctxt->call;

            found_call = clist_search(ret_call, pcall->callee->addr);
      
            printf("__exit_function: %s\n", poped_ctxt->func->name);

            DMSG("<----> (0x%08x to 0x%08x) [level %2d] < RET   %s  \n",
                 IL_INSN(prev_insn)->addr,
                 IL_INSN(curr_insn)->addr,
                 cs->stack->level,
                 poped_ctxt->func?poped_ctxt->func->name:"NULL");

            pcall->func = poped_ctxt->func;

            if(found_call == NULL){
               clist_add_head(&ret_call, pcall);
               pcall->costs->rep_counter++;
            }else{
               /* Update existing call costs */
               counters_sum(found_call->costs, ret_call->costs);
               found_call->costs->rep_counter++;
               call_free(pcall);
            }
            ctxt_free(poped_ctxt);
         }

         cs->prev_insn = curr_insn; // TODO: Remove this one !

         *call = ret_call;
         return CALLSTACK_RET;
      }
      /* ==================================================================== *
       * RETURNS (end)                                                        *
       * ==================================================================== */

      return CALLSTACK_NONE;
   }
   break;

   /***************************************************************************
    * Error State.                                                            *
    ***************************************************************************/
   case CS_STATE_ERROR:
      return CALLSTACK_ERROR;
      break;
   }

   /*
    * Point not reacheable.
    */
   cs->state = CS_STATE_ERROR;
   return CALLSTACK_ERROR;
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
