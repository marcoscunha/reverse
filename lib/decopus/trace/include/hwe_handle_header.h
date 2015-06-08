
#ifndef _HWE_HANDLE_HEADER_H_
#define _HWE_HANDLE_HEADER_H_

#include <stdbool.h>
#include <stdio.h>
#include <strings.h>

#include "hwe_handle.h"
#include "events/hwe_tools.h"

#include "hwe_handle_def.h"

/* 
 * At this point, some macros should be defined(or not if unused)
 * + EVENT_DATA_T: should be a type
 *                 is used in event_t structure
 * + COMP_DATA_T: should be a type
 *                is used in comp_t structure
 * + USE_STAGE_GO: if the stage 'go' is used
 * + USE_STAGE_RET: if the stage 'return' is used
 *
 * Implementation should look like this:
 *
 * | (#include "hwe_handle_def.h")
 * | 
 * | types/macro (non-)definition: EVENT/COMP_DATA_t & USE_STAGE_GO/RET
 * |
 * | #include "hwe_handle_main.h"
 * |
 * | implementation including functions definition: process_init/start/stop/info
 */

/*
 * An event is processed preprocessing in 3 stages executed in this order:
 *
 * + init: executed when the event is commited
 *         all references are valid (but maybe not commited)
 *         there may be missing followers (events which reference this one)
 *         for events issued by a same component, init are executed in issue order
 *
 * + go    : executed when all followers are commited
 *         executed before followers's 'go callbacks'
 *
 * + return: executed after followers's 'return callbacks'
 *
 * It is possible to enforce some additional callback execution order
 * between 2 events (Go/Ret cb executed before Go/Ret cb, the stages may be 
 * different).
 *
 * A callback is associated to each stage.
 * The 'init' callback have to be defined for each component, it is used for
 * each event issued by this component. It should be defined during 
 * 'process_component' call.
 * The 'go' and 'return' callbacks have to be defined for each event, they 
 * should be defined during the 'init' call.
 */

/*
 * These functions have to be defined in the implementation
 */
// called at beginning (before anything)
void process_init(const char *, int, char * const []);
// called for each HWE_INFO events at the beginning of the trace
void process_component(comp_t *, event_t *);
// called after HWE_INFO events and before non-HWE_INFO events
void process_start();
// called at the end
void process_stop();

/*
 * These functions are defined below and could/should be used
 */
static inline hwe_cont * event_content(event_t *e);
static inline comp_t * event_component(event_t *e);
static inline event_t * event_ref(event_t *e, int idx);
//static inline event_t * event_ref(event_t *e, int idx);
void event_print(FILE*,event_t *e);
//callback setter for 'init' stage for events issued by a component
static inline void set_cb_init(comp_t *c, void (*cb) (event_t *));
#ifdef EVENT_DATA_T
static inline EVENT_DATA_T * event_data(event_t *e);
#endif
#ifdef COMP_DATA_T
static inline COMP_DATA_T * comp_data(comp_t* c);
#endif
//components getter
unsigned get_maxcomponent();
comp_t * get_component(unsigned id);
// fifo init/putters
void evfifo_init(evfifo_t *f);
#ifdef USE_STAGE_GO
void put_infifo_go(evfifo_t *f, event_t *e);
void set_barrier_go_to_go(event_t *s, event_t *d);
//callback setter for 'go' stage
static inline void set_cb_go(event_t *e, void (*cb) (event_t *));
#endif
#ifdef USE_STAGE_RET
void put_infifo_ret(evfifo_t *f, event_t *e);
void set_barrier_ret_to_ret(event_t *s, event_t *d);
//callback setter for 'ret' stage
static inline void set_cb_ret(event_t *e, void (*cb) (event_t *));
#endif
#if defined(USE_STAGE_GO) && defined(USE_STAGE_RET)
void set_barrier_go_to_ret(event_t *s, event_t *d);
void set_barrier_ret_to_go(event_t *s, event_t *d);
#endif


#ifndef HWE_HANDLE_REPORT_LVL
#define HWE_HANDLE_REPORT_LVL 0
#endif

/****************************
 * Types & Pools Definition *
 ****************************/

/*
 * handler's side object corresponding to an event_t
 */
//preprocessing stage for stage 'go' and 'return'
struct event_stage {
   //callback of the stage
   void (*cb) (event_t *);
   //count of event blocking the execution of this stage
   int  cnt;
   // tells if the stage is already done
   bool done;
   //list of user-set followers
   evptr_t *usrfol;
};
struct event_t {
	//the event container
	// the container is allocated in a different place
	// in order to separe the event_t and hwe_cont in memory
	// since they may be manipulated by different threads
	hwe_cont *hwe;
   
	//the component which issued the event
   comp_t *comp;

	//list next
	event_t *next;

	//numbers of references/followers
	int nrefs;
   
	//tell if the event has been commited
   bool commited;
   
	//references missing count
   int missref;
   
	//followers list and missing count
   //(ie: event which references this event)
   int      missfol;
   evptr_t *followers;
  
	//stage
	struct event_stage init;
   struct event_stage go;
   struct event_stage ret;

#ifdef EVENT_DATA_T
   EVENT_DATA_T data;
#endif

#ifdef HANDLE_VERBOSE
	char desc[64];
#endif
#if HWE_HANDLE_REPORT_LVL >= 3
   event_t *rep_next;
   event_t *rep_prev;
#endif
#if HWE_HANDLE_REPORT_LVL >= 4
   event_t *rep_cnext;
   event_t *rep_cprev;
#endif
};

// component structure
struct comp_t {
   unsigned id;
   hwe_device_t type;
   void (*init)(event_t *);
   char *name;
	evfifo_t init_fifo;

#if HWE_HANDLE_REPORT_LVL >= 2
   unsigned long rep_nevhdl;
#endif
#if HWE_HANDLE_REPORT_LVL >= 4
   event_t *rep_head;
   event_t *rep_queue;
#endif
#if defined(HWE_HANDLE_DUMP) || HWE_HANDLE_REPORT_LVL >= 1
	hwe_id_ind_t lastid;
#endif

   //processing-specific data type
#ifdef COMP_DATA_T
   COMP_DATA_T data;
#endif
};

/*
 * inline functions
 */
__attribute__((__unused__))
static inline hwe_cont * event_content(event_t *e) {
	return e->hwe;
}
__attribute__((__unused__))
static inline comp_t * event_component(event_t *e) {
	return e->comp;
}
//callback setter for 'init' stage for events issued by a component
__attribute__((__unused__))
static inline void set_cb_init(comp_t *c, void (*cb) (event_t *)) {
	c->init = cb;
}
#ifdef EVENT_DATA_T
__attribute__((__unused__))
static inline EVENT_DATA_T * event_data(event_t *e) {
	return &e->data;
}
#endif
#ifdef COMP_DATA_T
__attribute__((__unused__))
static inline COMP_DATA_T * comp_data(comp_t* c)
{
	return &c->data;
}
#endif
__attribute__((__unused__))
static inline int event_nbref(event_t *e)
{
   return e->nrefs;
}
static event_t * event_mult_ref(hwe_head_cont *hwe, int idx)
{
	while (idx >= HWE_REF_MAX) {
		hwe = hwe->refnext;
		idx -= HWE_REF_MAX;
	}
	return (event_t *) hwe_getref(hwe, idx);
}
__attribute__((__unused__))
static inline event_t * event_ref(event_t *e, int idx)
{
	if (idx >= e->nrefs)
		return NULL;
	if (idx < HWE_REF_MAX) {
		return (event_t *) hwe_getref(&e->hwe->common, idx);
	}
	return event_mult_ref(e->hwe->common.refnext, idx - HWE_REF_MAX);
}

#ifdef USE_STAGE_GO
//callback setter for 'go' stage
__attribute__((__unused__))
static inline void set_cb_go(event_t *e, void (*cb) (event_t *))
{
	e->go.cb = cb;
}
#endif

#ifdef USE_STAGE_RET
//callback setter for 'ret' stage
__attribute__((__unused__))
static inline void set_cb_ret(event_t *e, void (*cb) (event_t *))
{
	e->ret.cb = cb;
}
#endif

/*
 * level in call stack of handler
 */
extern unsigned hwe_handle_lvl;

#endif /* _HWE_HANDLE_HEADER_H */
