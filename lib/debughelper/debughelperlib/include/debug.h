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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>

#ifndef DBG_HDR
#define DBG_HDR __FILE__
#endif

#if (defined HUGE_DEBUG) && !(defined DEBUG)
#define DEBUG
#endif

#ifndef DBG_FILE
#define DBG_FILE stderr
#endif

#ifndef DBG_HDR_SZ
#define DBG_HDR_SZ "15"
#endif

/* Error reporting macro */
#define EMSG(fmt, ...) fprintf(DBG_FILE, "[EE] (%-"DBG_HDR_SZ"s) " fmt,	\
                               DBG_HDR, ##__VA_ARGS__);
/* Simple debug messages reporting macro */
#ifdef DEBUG
# define DMSG(fmt, ...) fprintf(DBG_FILE, "(%-"DBG_HDR_SZ"s) " fmt,	\
                                DBG_HDR, ##__VA_ARGS__);
#else
# define DMSG(a...) do{}while(0)
#endif

#ifdef HUGE_DEBUG
# define HMSG(fmt, ...) fprintf(DBG_FILE, "(%-"DBG_HDR_SZ"s) " fmt,	\
                                DBG_HDR, ##__VA_ARGS__);
#else
# define HMSG(a...) do{}while(0)
#endif


/* Informative messages reporting */
# define IMSG(fmt, ...) fprintf(DBG_FILE, "(%-"DBG_HDR_SZ"s) " fmt,	\
                                DBG_HDR, ##__VA_ARGS__);

#endif /* _DEBUG_H */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */