#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdio.h>

#ifndef DBG_HDR
#define DBG_HDR __FILE__
#endif

#if (defined HUGE_DEBUG) && !(defined DEBUG)
#define DEBUG
#endif

#define DBG_HDR_SZ "17"

#ifndef __STRICT_ANSI_C__

/* Error reporting macro */
#define EMSG(fmt, ...) dna_printf( "[EE] (%s) " fmt,	\
			       DBG_HDR, ##__VA_ARGS__);

/* Simple debug messages reporting macro */
#ifdef DEBUG
#define DMSG(fmt, ...) dna_printf( "(%s) " fmt,	\
			       DBG_HDR, ##__VA_ARGS__);
#else
#define DMSG(a...) do{}while(0)
#endif

/* Heavy debug messages reporting macro */
#ifdef HUGE_DEBUG
#define HMSG(fmt, ...) dna_printf( "(%s) " fmt,	\
			       DBG_HDR, ##__VA_ARGS__);
#else
#define HMSG(a...) do{}while(0)
#endif

/* Informative messages reporting */
#define IMSG(fmt, ...) dna_printf( "(%s) " fmt,	\
			       DBG_HDR, ##__VA_ARGS__);

#else /* STRICT_ANSI */
/*
 * For crappy compilers purposes
 */

#ifdef DEBUG
#define __DBG__ 1
#else
#define __DBG__ 0
#endif

#ifdef HUGE_DEBUG
#define __HDBG__ 1
#else
#define __HDBG__ 0
#endif

/* Error reporting macro */
#define EMSG dna_printf

/* Simple debug messages reporting macro */
#define DMSG if(__DBG__) dna_printf

/* Heavy debug messages reporting macro */
#define HMSG if(__HMSG__) dna_printf

/* Informative messages reporting */
#define IMSG dna_printf

#endif /* STRICT_ANSI */

#endif				/* __DEBUG_H */
