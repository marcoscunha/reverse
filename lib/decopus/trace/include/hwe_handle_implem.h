
#include "events/hwe_tools.h"

/*****************
 * debugging macros *
 *****************/
#define HANDLE_PRINT(std, cat, fmt, ...) \
   fprintf((std), "<%s:%d><%s> " cat ":" fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define HANDLE_WARN(fmt, ...) HANDLE_PRINT(stderr, "WARNING", fmt, ##__VA_ARGS__)

#define HANDLE_ERROR(fmt, ...) HANDLE_PRINT(stderr, "ERROR", fmt, ##__VA_ARGS__)

#define HANDLE_ASSERT(cond, fmt, ...) do { \
   if (!(cond)) { \
      HANDLE_ERROR(fmt, ##__VA_ARGS__); \
      exit(EXIT_FAILURE); \
   } \
} while (0)

#define HANDLE_ASSERTEV(ev, cond, fmt, ...) do { \
   if (!(cond)) { \
      if ((ev)) { \
         char *pre = calloc(10U + ((ev->comp)?strlen(ev->comp->name):0), 1); \
         sprintf(pre, "<ERROR:%s> ", (ev->comp)?ev->comp->name:""); \
         hwe_print(stderr, ev->hwe, hwe_ref2id, pre, NULL); \
         free(pre); \
      } \
      HANDLE_ERROR(fmt, ##__VA_ARGS__); \
      exit(EXIT_FAILURE); \
   } \
} while (0)

#ifdef HANDLE_SAFEMODE
#define HANDLE_SAFECHECK(cond) HANDLE_ASSERT(cond, "Check failed")
#define HANDLE_SAFEASSERT(cond, fmt, ...) HANDLE_ASSERT(cond, fmt, ##__VA_ARGS__)
#define HANDLE_SAFEASSERTEV(ev, cond, fmt, ...) HANDLE_ASSERTEV(ev, cond, fmt, ##__VA_ARGS__)
#else
#define HANDLE_SAFECHECK(cond)
#define HANDLE_SAFEASSERT(cond, fmt, ...)
#define HANDLE_SAFEASSERTEV(ev, cond, fmt, ...)
#endif

unsigned hwe_handle_lvl = 0;

#define HANDLE_LVL_INC hwe_handle_lvl += 1
#define HANDLE_LVL_DEC hwe_handle_lvl -= 1

#ifdef HANDLE_VERBOSE
#define HANDLE_VERB_PRINTF(fmt,...) fprintf(stdout, fmt, ##__VA_ARGS__)

#define HANDLE_VPRINT(fmt, ...) { \
	HANDLE_VERB_PRINTF("[%c]HDL" fmt, (hwe_handle_lvl < 10) ? ((char)hwe_handle_lvl /* vlvl*/ + '0'): '+', ##__VA_ARGS__); \
}
#define HANDLE_VCALL(call, fmt, ...) { \
   HANDLE_VPRINT("<%s> " fmt "\n", call, ##__VA_ARGS__); \
}
#define HANDLE_VCALL_EV(call, ev) { \
   HANDLE_VCALL(call, "%s", ev->desc); \
}
#else
#define HANDLE_VPRINT(fmt, ...)
#define HANDLE_VCALL(call, fmt, ...)
#define HANDLE_VCALL_EV(call, ev)
#endif

//pool of hwe container
#define POOL_prefix hwe_pool
#define POOL_elem_t hwe_cont
#define POOL_grain 1000
#define POOL_STAT
#include "../utils/include/pool.h"

//pool of event
#define POOL_prefix event_pool
#define POOL_elem_t event_t
#define POOL_grain 1000
#define POOL_STAT
#include "../utils/include/pool.h"

//pool of list elements
#define POOL_prefix evptr_pool
#define POOL_elem_t evptr_t
#define POOL_grain 100
#define POOL_STAT
#include "../utils/include/pool.h"

/*
 * give an hwe_id_t from a hwe_ref_t
 * used for print
 */
__attribute__((__unused__)) static hwe_id_t hwe_ref2id(hwe_ref_t ref)
{
	hwe_id_t id = HWE_ID_NULL;
	if (ref == HWE_REF_NULL)
		return id;
	return ((event_t *)ref)->hwe->common.id;
}

void event_print(FILE *f, event_t *e)
{
	hwe_print(f, e->hwe, hwe_ref2id, NULL, NULL); \
}

#ifdef HWE_HANDLE_DUMP

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int dump_fd = -1;
#define BUFFERSIZE 1000000
static void *dump_buffer = NULL;
static size_t dump_bufsize = 0;

/*
 * buffer flush to the file
 */
static void dump_flush()
{
   write(dump_fd, dump_buffer, dump_bufsize);
   dump_bufsize = 0;
}

/*
 * write 1 event into the buffer
 */
static void dump_event(event_t *e)
{
   hwe_cont *hwe = e->hwe;
   comp_t *c = e->comp;
	hwe_id_t ids[HWE_REF_MAX];

   // write main event into the buffer
   {
		for (unsigned i = 0; i < hwe_getnref(&hwe->common); i += 1) {
			ids[i] = hwe_ref2id(hwe_getref(&hwe->common, i));
		}

      size_t size = hwe_sizeof(hwe);
      if (dump_bufsize + size + HWE_ID_SIZEOF > BUFFERSIZE)
         dump_flush();

      // if necessary insert a HWE_ID
      if (!hwe_head_rid_compute(&hwe->common, c->lastid)) {
         hwe_id_write(&hwe->common, dump_buffer + dump_bufsize);
         dump_bufsize += HWE_ID_SIZEOF;
      }

      hwe_write(hwe, &ids[0], dump_buffer + dump_bufsize);
      dump_bufsize += size;
      
      //update id
      c->lastid = hwe->common.id.index;
   }
   
   // write additionals containers
   while ((hwe = ((hwe_cont *) hwe->common.refnext))) {
		for (unsigned i = 0; i < hwe_getnref(&hwe->common); i += 1) {
			ids[i] = hwe_ref2id(hwe_getref(&hwe->common, i));
		}
      
      size_t size = hwe_sizeof(hwe);
      if (dump_bufsize + size > BUFFERSIZE)
         dump_flush();

      //set rid to 0 (ie: same event)
      hwe_head_rid_zero(&hwe->common);

      hwe_write(hwe, &ids[0], dump_buffer + dump_bufsize);
      dump_bufsize += size;
   }
}

/*
 * init
 */
static void dump_init()
{

   //open file
   dump_fd = creat("handler.hwe", S_IRUSR | S_IWUSR);

   //init buffer
   dump_bufsize = 0;
   dump_buffer = malloc(BUFFERSIZE);
}

/*
 * stop
 */
static void dump_stop()
{
   //flush buffer, close & free things
   dump_flush();
   close(dump_fd);
   free(dump_buffer);
}

/*
 * info
 */
static void dump_component(comp_t *comp, event_t *e)
{
	comp->lastid = 0;
   //put the event into the file
   dump_event(e);
}
#endif

/********************
 * Global variables *
 ********************/

// number of components (will be updated when needed)
static unsigned maxcomp = 16;
// component array
static comp_t **comp = NULL;
// events things
static struct {
	// total number of allocated events
	unsigned nb;
	// list of unused event
	event_t *list;
	//pools
	hwe_pool_t   hwe_pool;
	event_pool_t event_pool;
} events;
// pool of list element
static evptr_pool_t ptrpool;

#if HWE_HANDLE_REPORT_LVL >= 1
static unsigned long rep_nevhdl;
#endif
#if HWE_HANDLE_REPORT_LVL >= 2
static unsigned rep_nevcur;
#endif
#if HWE_HANDLE_REPORT_LVL >= 3
static event_t *rep_head, *rep_queue;
#endif


/*******************
 * Global accesser *
 *******************/
__attribute__((__unused__))
unsigned get_maxcomponent()
{
   return maxcomp;
}
__attribute__((__unused__))
comp_t * get_component(unsigned id)
{
   if (id >= maxcomp)
      return NULL;
   return comp[id];
}

/***********************
 * Event new & recycle *
 ***********************/
/*
 * default callback
 */
static void cb_epicfail(event_t *e)
{
   const char *stage = "init";
   if (e->commited)
      stage = "go";
   if (e->go.done)
      stage = "ret";
   HANDLE_ERROR("%s's callback not set for:\n", stage);
   hwe_print(stderr, e->hwe, hwe_ref2id, NULL, NULL);
   exit(EXIT_FAILURE);
}

/**
 * @brief get a new event structure
 * @param e
 */
static inline void event_init (event_t *e)
{
	struct event_stage tmp = {
		.cb = cb_epicfail,
		.cnt = 0,
		.done = false,
		.usrfol = NULL
	};
   e->comp = NULL;
   e->next = NULL;
	e->nrefs = -1;
   e->missfol = 0;
   e->followers = NULL;
   e->commited = false;
   e->missref = 0;
	e->init = tmp;
	e->go = tmp;
	e->ret = tmp;
#if HWE_HANDLE_REPORT_LVL >= 3
   e->rep_next = NULL;
   e->rep_prev = NULL;
#endif
#if HWE_HANDLE_REPORT_LVL >= 4
   e->rep_cnext = NULL;
   e->rep_cprev = NULL;
#endif
}

/**
 *
 * @return
 */
static inline event_t * event_new() {
	event_t *e = NULL;

	if (events.list) {
		e = events.list;
		events.list = e->next;
#ifdef HWE_HANDLE_MAXEVENT
	} else if (events.nb >= HWE_HANDLE_MAXEVENT) {
		return NULL;
#endif
	} else {
             events.nb += 1U;
             e = event_pool_get(&events.event_pool);
             e->hwe = hwe_pool_get(&events.hwe_pool);
             e->hwe->common.self = (hwe_ref_t) e;
	}
   
	HANDLE_SAFEASSERT(((event_t *)e->hwe->common.self) == e, "bad link between event and hwe");

	event_init(e);

#if defined(HANDLE_SAFEMODE) && defined(EVENT_DATA_T)
   memset(&e->data, 0, sizeof(e->data));
#endif

   return e;
}

/**
 * @brief Put back a used event
 * @param e
 */
static void event_recycle(event_t *e)
{
	e->next = events.list;
	events.list = e;
}

/****************
 * HANDLE START *
 ****************/

void handle_start(const char *tracename, int nopt, char * const opt[]) {
   // alloc and init at 0
   comp = calloc(maxcomp, sizeof(comp_t *));
   hwe_pool_init(&events.hwe_pool);
   event_pool_init(&events.event_pool);
   evptr_pool_init(&ptrpool);
	events.list = NULL;
	events.nb = 0U;
	// the HWE_REF_NULL

#if HWE_HANDLE_REPORT_LVL >= 3
   rep_head = NULL;
   rep_queue = NULL;
#endif
  
#ifdef HWE_HANDLE_DUMP
	dump_init();
#endif

	/*processing-specific*/
   process_init(tracename, nopt, opt);
}

/*****************
 * HANDLE REPORT *
 *****************/
__attribute__((__unused__))
static void handle_report() {
#if HWE_HANDLE_REPORT_LVL == 1
   fprintf(stderr, "HWE_handle report: %lu events\n", rep_nevhdl);
#elif HWE_HANDLE_REPORT_LVL >= 2
   fprintf(stderr, "HWE_handle report:\n");
   fprintf(stderr, " -> %lu events (", rep_nevhdl);
   {
      bool first = true;
      for (unsigned c = 0; c < maxcomp; c++) {
         if (comp[c]) {
            fprintf(stderr, "%s%u:%lu", first ? "" :",", c, comp[c]->rep_nevhdl);
            first = false;
         }
      }
   }
   fprintf(stderr, ")\n -> %u still in processing\n", rep_nevcur);
#if HWE_HANDLE_REPORT_LVL >= 3
   {
      unsigned mref = 0, mfol = 0;
      for (event_t *e = rep_queue; e != NULL; e = e->rep_next) {
         if (e->missref != 0)
            mref += 1;
         if (e->missfol != 0)
            mfol += 1;
      }
      fprintf(stderr, " -> %u with missing ref\n", mref);
      fprintf(stderr, " -> %u with missing fol\n", mfol);
   }
   {
      for (event_t *e = rep_queue; e != NULL; e = e->rep_next) {
         char desc[70];
         hwe_desc(e->hwe, hwe_ref2id, &desc[0], 70);
         desc[69] = '\0';
         unsigned nref = 0;
         for (hwe_cont *cur = e->hwe; cur != NULL; cur = (hwe_cont *) cur->common.refnext)
            nref += hwe_getnref(&cur->common);
         unsigned nfol = e->hwe->common.head.expected;
         fprintf(stderr, " -> (%c|R%d/%u|F%d/%u) %s\n", 
               e->go.done ? 'G' : 'C',
               e->missref, nref,
               e->missfol, nfol,
               desc);
      }
   }
#endif
#endif
}


/***************
 * HANDLE STOP *
 ***************/
void handle_stop() {
#ifdef HWE_HANDLE_DUMP
	dump_stop();
#endif
   
   /*stop processing*/
   process_stop();
 
   handle_report();

   //stop and free things
   for (unsigned i = 0; i < maxcomp; i++) {
      if (comp[i]) {
         free(comp[i]->name);
         free(comp[i]);
      }
   }
   free(comp);
   hwe_pool_free(&events.hwe_pool);
   event_pool_free(&events.event_pool);
   evptr_pool_free(&ptrpool);
}

/*******************
 * HANDLE ALLOCATE *
 *******************/
hwe_cont * handle_alloc()
{
   //allocate an event
   return event_new()->hwe;
}

/****************
 * HANDLE EVENT *
 ****************/
static inline void rm_frfifo(evfifo_t *fifo, event_t *ev);
static inline void put_infifo(evfifo_t *fifo, event_t *ev, enum stage_t stg);
static void check_init(event_t *e);
static void check_go(event_t *e);
static void check_ret(event_t *e);
/**
 *
 * @param hwe
 */
void handle_event(hwe_cont *hwe)
{
	// boolean used to shortcuts the HWE_INFO handling when its done
   static bool start_done = false;
	
   event_t *e = (event_t *) hwe->common.self;
	HANDLE_ASSERT(e->hwe == hwe, "Fatal error: invalid link");
#if HWE_HANDLE_REPORT_LVL >= 1
   rep_nevhdl += 1;
#endif
#ifdef HWE_HANDLE_TRACE
	hwe_print(stdout, e->hwe, hwe_ref2id, NULL, NULL);
#endif

   //hwe_print(stderr, hwe, hwe_ref2id, NULL, NULL);
   if (!start_done) {
      // trace must start by all HEV_INFO events
      if (hwe->common.head.type == HWE_INFO) {
         unsigned id = hwe->common.id.devid;
         hwe_info_cont *info = &hwe->info;
         
         // ensure correct HWE_INFO
         if (hwe->common.head.nrefs) {
            HANDLE_WARN("Bad HWE_INFO, it has reference(s)");
         }
			if (hwe->common.head.expected) {
            HANDLE_WARN("Bad HWE_INFO, it expects to be referenced");
			}

         
         //register component
         if (maxcomp <= id) {
            comp_t **newtab = calloc(maxcomp + maxcomp, sizeof(comp_t*));
            memcpy(newtab, comp, sizeof(comp_t*) * maxcomp);
            free(comp);
            comp = newtab;
            maxcomp *= 2;
         }

         //hwe_print(stderr, hwe, hwe_ref2id, NULL, NULL);
         if (!comp[id]) {
            // create a new component
            comp_t *c = calloc(1, sizeof(comp_t));
            comp[id] = c;
            c->id = id;
            c->type = info->body.device;
            c->init = cb_epicfail;
            evfifo_init(&c->init_fifo);
            c->name = calloc(1U + info->body.nsize, 1);
            strcpy(c->name, info->name);
#if HWE_HANDLE_REPORT_LVL >= 2
            c->rep_nevhdl = 1;
#endif
#if HWE_HANDLE_REPORT_LVL >= 4
            c->rep_head = NULL;
            c->rep_queue = NULL;
#endif
         } else {
            hwe_print(stderr, hwe, hwe_ref2id, NULL, NULL);
            HANDLE_ASSERT(false, "Multiple HWE_INFO for the same component");
         }

         // specific-processing init
         e->comp = comp[id];
#ifdef HWE_HANDLE_DUMP
			dump_component(comp[id], e);
#endif
         process_component(comp[id], e);

         //recycle this event
         event_recycle(e);
          
         return;
      }
      
      //specific-processing start
      process_start();
      
      //start things before handling 1st real event   
      start_done = true;

   }

   //register event
   
#ifdef HWE_SAFEMODE
   {
      unsigned id = hwe->common.id.devid;
      if (id >= maxcomp || !comp[id]) {
         hwe_print(stderr, hwe, hwe_ref2id, "<ERROR> ", NULL);
         HANDLE_ERROR("Bad event's component id\n");
      }
   }
#endif
   comp_t *c = comp[hwe->common.id.devid];
   e->comp = c;

	{
		int refcnt = 0;
		for (hwe_cont *cur = hwe; cur != NULL; cur = (hwe_cont *) cur->common.refnext) {
			refcnt += hwe_getnref(&cur->common);
			HANDLE_SAFEASSERT((cur->common.head.type == e->hwe->common.head.type) ||
							  (cur->common.head.type == HWE_NULL), "Inconsistent container type");
#ifdef HWE_HANDLE_SAFEMODE
			for (unsigned i = 0; i < hwe_getnref(&cur->common); i++) {
				hwe_ref_t ref = hwe_getref(&cur->common, i);
				HANDLE_SAFEASSERT(ref != HWE_REF_NULL, "Null reference");
			}
#endif
		}
		e->nrefs = refcnt;
	}
   
   //insert in trace order
   //At this point all references of _e_ are valid pointers
   //But the content of the references may not be valid (because
   //they may not be commited)
   //So only the _id_ of theses references may be read safely

#ifdef HANDLE_VERBOSE
   hwe_desc(e->hwe, hwe_ref2id, e->desc, 60);
   e->desc[59] = '\0';
#endif
   HANDLE_VCALL_EV("trace", e);
   HANDLE_LVL_INC;
#ifdef HWE_HANDLE_DUMP
	dump_event(e);
#endif
#ifdef EVENT_DATA_T
	memset(&e->data, 0, sizeof(EVENT_DATA_T));
#endif
	e->init.cb = e->comp->init;
	put_infifo(&e->comp->init_fifo, e, STAGE_INIT);

#if HWE_HANDLE_REPORT_LVL >= 2
   rep_nevcur += 1;
   e->comp->rep_nevhdl += 1;
#endif
#if HWE_HANDLE_REPORT_LVL >= 3
   e->rep_next = NULL;
   e->rep_prev = rep_head;
   if (rep_head)
      rep_head->rep_next = e;
   else
      rep_queue = e;
   rep_head = e;
#endif
#if HWE_HANDLE_REPORT_LVL >= 4
   e->rep_cnext = NULL;
   e->rep_cprev = e->comp->rep_head;
   if (e->comp->rep_head)
      e->comp->rep_head->rep_cnext = e;
   else
      e->comp->rep_queue = e;
   e->comp->rep_head = e;
#endif

   //update our missfol count
   e->missfol += hwe->common.head.expected;

   // loop on references
   // Warning: due to the check_go call in the loop, references may be deleted in the loop
   for (hwe_cont *cur = hwe; cur != NULL; cur = (hwe_cont *) cur->common.refnext) {
      for (unsigned i = 0; i < hwe_getnref(&cur->common); i++) {
         event_t *ref = (event_t *) hwe_getref(&cur->common, i);

			/*
			 * increment our missing count
			 * only if the ref has not been commited
			 */
			if (!ref->commited) {
				e->missref += 1;
			}

         /*
			 * add reverse ref pointer in the referenced event 
			 * since it is for check_go stage
			 * only do it if it not already done
			 */
			if (!ref->go.done) {
				evptr_t *ptr = evptr_pool_get(&ptrpool);
				ptr->event = e;
				ptr->next = ref->followers;
				ref->followers = ptr;
#ifdef USE_STAGE_GO
				e->go.cnt += 1;
#endif
			}

         //update ref follower missing count
         ref->missfol -= 1;
#ifdef USE_STAGE_RET
         //increment reference 'ret' counter
         ref->ret.cnt += 1;
#endif
      }
   }

   //update followers missref counts
	//(they reference this event and have been commited before)
   for (evptr_t *ptr = e->followers; ptr != NULL; ptr = ptr->next) {
      ptr->event->missref -= 1;
      check_init(ptr->event);
   }
   
	//we just indicate the commit here
   //in order to be sure the event survive the calls
   e->commited = true;
	check_init(e);
   HANDLE_LVL_DEC;
}

/**********
 * Checks *
 **********/
static void free_barrier(evptr_t *ptr)
{
#if defined(HANDLE_VERBOSE) && HANDLE_VERBOSE >= 1
	HANDLE_VCALL(__func__, "%u.%"HWE_PRI_ID".%s", 
			ptr->event->hwe->common.id.devid, ptr->event->hwe->common.id.index,
			(ptr->stage == STAGE_INIT) ? "init" : (ptr->stage == STAGE_GO) ? "go" : "ret"); 
   HANDLE_LVL_DEC;
#endif	
	/*
	 * barrier
	 */
	switch (ptr->stage) {
		case STAGE_INIT:
			ptr->event->init.cnt -= 1;
			check_init(ptr->event);
			break;
		case STAGE_GO:
			ptr->event->go.cnt -= 1;
			check_go(ptr->event);
			break;
		case STAGE_RET:
			ptr->event->ret.cnt -= 1;
			check_ret(ptr->event); 
			break;
		case STAGE_TRACE:
			break;
	}
}

/*
 * try to call 'init' callback
 */
static void check_init(event_t *e)
{
	if (!e->commited || e->missref != 0)
		return;

	if (e->init.cnt != 0)
		return;

   //the 'go' callback
   HANDLE_VCALL_EV("init",  e);
   HANDLE_LVL_INC;
   e->init.cb(e);
   
	//unlock waiting events
   {
      evptr_t *ptr = e->init.usrfol;
      while (ptr) {
        	evptr_t *next = ptr->next;
			if (ptr->event) {
				free_barrier(ptr);
			} else {
				rm_frfifo(ptr->fifo, e);
			}
			evptr_pool_put(&ptrpool, ptr);
			ptr = next;
      }
      e->init.usrfol = NULL;
   }
   
	e->init.done = true;
   check_go(e);
   HANDLE_LVL_DEC;
}

/*
 * try to call 'go' callback
 */
static void check_go(event_t *e)
{
   //we need the event to be:
   // + init done (and missref == 0)
   // + all previous go done
   if (!e->init.done)
      return;

#ifdef USE_STAGE_GO
   if (e->go.cnt != 0)
      return;

   //the 'go' callback
   HANDLE_VCALL_EV("go",  e);
   HANDLE_LVL_INC;
   e->go.cb(e);
   
   //unlock waiting events
   {
      evptr_t *ptr = e->go.usrfol;
      while (ptr) {
        	evptr_t *next = ptr->next;	
			if (ptr->event) {
				free_barrier(ptr);
			} else {
				rm_frfifo(ptr->fifo, e);
			}
			evptr_pool_put(&ptrpool, ptr);
			ptr = next;
      }
      e->go.usrfol = NULL;
   }
#endif
   
   //loop on reverse references (followers: which event does a reference on _e_ ?)
   {
      evptr_t *ptr = e->followers;
      while (ptr) {

#ifdef USE_STAGE_GO
         //update and check follower event
         ptr->event->go.cnt -= 1;
         check_go(ptr->event);
#endif

         //next one in the list (and recycle current ptr)
         evptr_t *tmp = ptr;
         ptr = ptr->next;
         evptr_pool_put(&ptrpool, tmp);
      }
      e->followers = NULL;
   }

   //check the 'ret' callback
   e->go.done = true;
   check_ret(e);
#ifdef USE_STAGE_GO
   HANDLE_LVL_DEC;
#endif
}

/*
 * try to call the 'ret' callback
 */
static void check_ret(event_t *e)
{
//   printf("a\n");
   // we need
   // + the 'go' callback to be called
   // + all followers need to be here (retcnt isn't valid be fore that point)
   // + _retcnt_ == 0 (ei: the 'ret' of each follower has been called)
   if (!e->go.done || e->missfol != 0)
      return;

#ifdef USE_STAGE_RET
   if (e->ret.cnt != 0)
      return;

   // the 'ret' callback
   HANDLE_VCALL_EV("ret", e);
   HANDLE_LVL_INC;
   e->ret.cb(e);
#define SHERLOCK
#ifdef SHERLOCK
   //loop on references
   //and recycle this event
	for (hwe_cont *cur = e->hwe; cur != NULL; cur = (hwe_cont *) cur->common.refnext) {
#ifdef USE_STAGE_RET
      for (unsigned i = 0; i < cur->common.head.nrefs; i++) {
         event_t *ref = (event_t *) cur->common.refs[i];
         //update and check reference
         ref->ret.cnt -= 1;
         check_ret(ref);
      }
#endif

      //delete the container
		//note: the loop iteration works fine because
		//this call does not modify "cur"
      event_recycle((event_t *)cur->common.self);
   }
#endif


   //unlock waiting events
   evptr_t *ptr = e->ret.usrfol;
   while (ptr) {
		evptr_t *next = ptr->next;	
		if (ptr->event) {
			free_barrier(ptr);
		} else {
			rm_frfifo(ptr->fifo, e);
		}
		evptr_pool_put(&ptrpool, ptr);
		ptr = next;
   }
   e->ret.usrfol = NULL;
#endif

#if HWE_HANDLE_REPORT_LVL >= 2
   rep_nevcur -= 1;
#endif
#if HWE_HANDLE_REPORT_LVL >= 3
   if (e->rep_next)
      e->rep_next->rep_prev = e->rep_prev;
   else
      rep_head = e->rep_prev;
   if (e->rep_prev)
      e->rep_prev->rep_next = e->rep_next;
   else
      rep_queue = e->rep_next;
#endif
#if HWE_HANDLE_REPORT_LVL >= 4
   if (e->rep_cnext)
      e->rep_cnext->rep_cprev = e->rep_cprev;
   else
      e->comp->rep_head = e->rep_cprev;
   if (e->rep_cprev)
      e->rep_cprev->rep_cnext = e->rep_cnext;
   else
      e->comp->rep_queue = e->rep_cnext;
#endif
#ifndef SHERLOCK
   //loop on references
   //and recycle this event
	for (hwe_cont *cur = e->hwe; cur != NULL; cur = (hwe_cont *) cur->common.refnext) {
#ifdef USE_STAGE_RET
      for (unsigned i = 0; i < cur->common.head.nrefs; i++) {
         event_t *ref = (event_t *) cur->common.refs[i];
         //update and check reference
         ref->ret.cnt -= 1;
         check_ret(ref);
      }
#endif

      //delete the container
		//note: the loop iteration works fine because
		//this call does not modify "cur"
      event_recycle((event_t *)cur->common.self);
   }
#endif
#ifdef USE_STAGE_RET
   HANDLE_LVL_DEC;
#endif
}

/************************************
 * Order/Barrier and fifo functions *
 ************************************/

/*
 * initialize a fifo
 */
__attribute__((__unused__))
void evfifo_init(evfifo_t *f)
{
   f->queue = NULL;
}

/*
 * generic internal function to add a barrier between 2 stages of 2 events
 */
static inline void set_barrier(
      event_t *evsrc, enum stage_t stsrc,
      event_t *evdst, enum stage_t stdst)
{
#if defined(HANDLE_VERBOSE) && HANDLE_VERBOSE >= 1
   HANDLE_VCALL(__func__, "%u.%"HWE_PRI_ID".%s -> %u.%"HWE_PRI_ID".%s", 
         evsrc->hwe->common.id.devid, evsrc->hwe->common.id.index,
         (stsrc == STAGE_INIT) ? "init" : (stsrc == STAGE_GO) ? "go" : "ret", 
         evdst->hwe->common.id.devid, evdst->hwe->common.id.index,
         (stdst == STAGE_INIT) ? "init" : (stdst == STAGE_GO) ? "go" : "ret"); 
#endif	
   //create list element
   evptr_t *elem = evptr_pool_get(&ptrpool);                               
   elem->fifo = NULL;
   elem->event = evdst;
   elem->stage = stdst;
   //increment stage cnt of dst event
	switch(stdst) {
		case STAGE_INIT:
			evdst->init.cnt += 1;
			break;
		case STAGE_GO:
			evdst->go.cnt += 1;
			break;
		case STAGE_RET:
			evdst->ret.cnt += 1;
			break;
		case STAGE_TRACE:
			break;
	}
   //add it to the list
	switch(stsrc) {
		case STAGE_INIT:
			elem->next = evsrc->init.usrfol;
			evsrc->init.usrfol = elem;
			break;
		case STAGE_GO:
			elem->next = evsrc->go.usrfol;
			evsrc->go.usrfol = elem;
			break;
		case STAGE_RET:
			elem->next = evsrc->ret.usrfol;
			evsrc->ret.usrfol = elem;
			break;
		case STAGE_TRACE:
			break;
	}
}

/*
 * generic internal function to put a stage in a fifo
 */
static inline void put_infifo(evfifo_t *fifo, event_t *ev, enum stage_t stg)
{
#if defined(HANDLE_VERBOSE) && HANDLE_VERBOSE >= 1
   HANDLE_VCALL(__func__, "%u.%"HWE_PRI_ID".%s",
         ev->hwe->common.id.devid, ev->hwe->common.id.index,
         (stg == STAGE_INIT) ? "init" : (stg == STAGE_GO) ? "go" : "ret");
#endif

   //if there is already an event in the fifo, then add a barrier
   if (fifo->queue) {
      set_barrier(fifo->queue, fifo->stage, ev, stg);
   }

   //update fifo queue info
   fifo->queue = ev;
   fifo->stage = stg;

   // add an empty element to the (new) queue list 
   // (it will be used to delete the fifo queue info if the event is deleted
   // before being replaced as fifo queue)
   evptr_t *elem = evptr_pool_get(&ptrpool);
   elem->event = NULL;//so we now it is special
   //elem->stage: don't care
   elem->fifo = fifo;
	switch(stg) {
		case STAGE_TRACE:
			break;
		case STAGE_INIT:
			elem->next = ev->init.usrfol;
			ev->init.usrfol = elem;
			break;
		case STAGE_GO:
			elem->next = ev->go.usrfol;
			ev->go.usrfol = elem;
			break;
		case STAGE_RET:
			elem->next = ev->ret.usrfol;
			ev->ret.usrfol = elem;
			break;
	}
}

/*
 * called to update a fifo after an event is removed from it
 */
static inline void rm_frfifo(evfifo_t *fifo, event_t *ev)
{
#if defined(HANDLE_VERBOSE) && HANDLE_VERBOSE >= 1
	HANDLE_VCALL(__func__, "%u.%"HWE_PRI_ID,
			ev->hwe->common.id.devid, ev->hwe->common.id.index);
#endif	
   //update
   if (fifo->queue == ev)
      fifo->queue = NULL;
}

#ifdef USE_STAGE_GO
/*
 * set a barrier between 2 'go' callbacks
 */
__attribute__((__unused__))
void set_barrier_go_to_go(event_t *s, event_t *d)
{
   set_barrier(s, STAGE_GO, d, STAGE_GO);
}

/*
 * put a 'go' callback into a fifo
 */
__attribute__((__unused__))
void put_infifo_go(evfifo_t *f, event_t *e)
{
   put_infifo(f, e, STAGE_GO);
}
#endif

#ifdef USE_STAGE_RET
/*
 * set a barrier between 2 'ret' callbacks
 */
__attribute__((__unused__))
inline void set_barrier_ret_to_ret(event_t *s, event_t *d)
{
   set_barrier(s, STAGE_RET, d, STAGE_RET);
}

/*
 * put a 'ret' callback into a fifo
 */
__attribute__((__unused__))
void put_infifo_ret(evfifo_t *f, event_t *e)
{
   put_infifo(f, e, STAGE_RET);
}
#endif

#if defined(USE_STAGE_GO) && defined(USE_STAGE_RET)
/*
 * set a barrier between a 'go' callbacks and a 'ret' callback
 */
__attribute__((__unused__))
inline void set_barrier_go_to_ret(event_t *s, event_t *d)
{
   set_barrier(s, STAGE_GO, d, STAGE_RET);
}

/*
 * set a barrier between a 'ret' callbacks and a 'go' callback
 */
__attribute__((__unused__))
inline void set_barrier_ret_to_go(event_t *s, event_t *d)
{
   set_barrier(s, STAGE_RET, d, STAGE_GO);
}
#endif

