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
#include "counters.h"

#ifdef COUNTERS_DEBUG
#define DEBUG
#endif

#define DBG_HDR "counters"
#include <debug.h>

counters_t *
counters_new(void) {

	counters_t *res = (counters_t *)malloc(sizeof(counters_t));

	res->rep_counter = 0;

	res->total_ir    = 0;
	res->total_en    = 0;
	res->total_cyc   = 0;
	res->nb_IC_miss  = 0;
	res->nb_DC_miss  = 0;
	res->nb_IT_miss  = 0;
	res->nb_DT_miss  = 0;

	return res;
}

void
counters_free(counters_t *count){
  
	free(count);
	return;
}

void
counters_sum(counters_t *a, counters_t *b) {

	/* a->rep_counter = 0; */
	/* a->total_ir   += b->total_ir * b->rep_counter; */
	a->total_ir   += b->total_ir;
	a->total_en   += b->total_en;
	a->total_cyc  += b->total_cyc;
	a->nb_IC_miss += b->nb_IC_miss;
	a->nb_DC_miss += b->nb_DC_miss;
	a->nb_IT_miss += b->nb_IT_miss;
	a->nb_DT_miss += b->nb_DT_miss;

	return;
} 

void
counters_clear(counters_t *count) {

	count->rep_counter = 0;
	count->total_ir    = 0;
	count->total_en    = 0;
	count->total_cyc   = 0;
	count->nb_IC_miss  = 0;
	count->nb_DC_miss  = 0;
	count->nb_IT_miss  = 0;
	count->nb_DT_miss  = 0;

}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
