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

/**
 * @file   insn.c
 * @author  <nfournel@bans>
 * @date   Mon Oct 20 16:09:43 2008
 * 
 * @brief  Instruction handling helpers.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <insn.h>
#include <counters.h>

#ifdef INSN_DEBUG
#define DEBUG
#endif

#define DBG_HDR "insn"
#include <debug.h>

/** 
 * Create a new instruction structure.
 * 
 * @param pc Program counter of the instruction.
 * 
 * @return The created instruction. 
 */
insn_t *
insn_new(pc_t pc){

	insn_t *res = (insn_t *)malloc(sizeof(insn_t));

	res->addr     = pc;
	res->flags    =  0;

	res->func     = NULL; 
	res->line     = NULL;
	res->calls    = NULL;
	res->fname    = NULL;

	res->ret_addr = 0x0;

	res->costs    = counters_new();

	return res;
}

/** 
 * Destroy an instruction structure.
 * 
 * @param pinsn Instruction to be destoyed.
 */
void
insn_free(insn_t *pinsn){

	clist_free(pinsn->calls);
	counters_free(pinsn->costs);
	free(pinsn);
	return;
}

/** 
 * Debugging function : dumps the structure content
 * 
 * @param pinsn Instruction structure to be dumped.
 */
void
insn_dump(insn_t *pinsn){

	IMSG("%s%s@0x%08x\n",
	     (pinsn->flags & INSN_FLAGS_UNUSED)?"~":" ",
	     (pinsn->flags & INSN_FLAGS_NEW_LINE)?"*":" ",
	     (unsigned int)pinsn->addr);
	if(pinsn->flags & INSN_FLAGS_FUNC_ENTRY){
		function_t *func = pinsn->func;
		if(func->name){
			IMSG("   func:%s\n", func->name);
		}else{
			IMSG("   func:<NONAME>\n");
		}
	}
	if(pinsn->flags & INSN_FLAGS_NEW_LINE){
		line_nfo_t *line = pinsn->line;
		if(line){
			IMSG("   line:%d:%s\n", line->lineno, pinsn->fname);
		}

	}
	if(pinsn->flags & INSN_FLAGS_FUNC_CALL){
		call_t *calls = pinsn->calls;
		if(calls){
			IMSG("   call:%s\n", calls->callee->func->name);
		}
	}

	return;
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
