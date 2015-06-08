#ifndef _HWETRACE_H_
#define _HWETRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * hwetrace API
 */
#include "hwetrace_api.h"

/*
 * event setters can be found in these files
 * "hwetrace_common.h"
 * "hwetrace_processor.h"
 * "hwetrace_memory.h"
 * "hwetrace_cache.h"
 */

/*
 * MEMORY REQUEST TRACE LEVEL
 * + 0: nothing
 * + 1: cpu request with no ack
 * + 2: evrything
 */
#ifndef HWETRACE_IMEM
#define HWETRACE_IMEM 2
#endif
#ifndef HWETRACE_DMEM
#define HWETRACE_DMEM 2
#endif


#ifdef __cplusplus
}
#endif

#endif // _HWETRACE_H_

