
#include <stdio.h>
#include <assert.h>


#include "hwetrace.h"
#define HWETRACE_DEBUG_IMPLEM
#include "hwetrace_common.h"
#undef HWETRACE_DEBUG_IMPLEM

//#define DEBUG

#define HWETRACE_PRINT(fmt, ...) fprintf(stderr,"[HWETRACE] " fmt, ## __VA_ARGS__)

#include "hwe_handle.h"

#include "hwetrace_stat.h"
#include "hwetrace_stat_internal.h"

/*
 * parallel things
 */
#ifdef HWETRACE_PARALLEL
#include "hwetrace_par_cirbuf.h"
#endif

/*
 * Internal trace type
 */
#define MAXPORTS 256
typedef struct hwe_trace_t {
   char        *name;
   unsigned     nports;//number of currently opened ports
   hwe_port_t **ports;
	uint64_t     nevents;
	HWETRACE_TRACESTAT_FIELD(trstat);
#ifdef HWETRACE_DUMP
	FILE *dump;
#endif
#ifdef HWETRACE_PARALLEL
	struct hwepar hwepar;
#endif
} hwe_trace_t;

static const char *const default_name =  "trace";

static hwe_trace_t trace = {
   .name   = NULL,
   .nports = 0,
   .ports  = NULL,
	.nevents = 0
};

/*
 * Port structure
 */
struct hwe_port_t {
   char          name[32];
   hwe_id_ind_t  index;
   hwe_id_dev_t  devid;
	hwe_device_t  type;
	HWETRACE_TRACESTAT_FIELD(trstat);
};

/**************
 * open trace *
 **************/
extern void hwetrace_open (const char * name)
{
   char *n = malloc(32);
   strncpy(n, name, 32);
   n[31] = '\0';
   trace.name = n;
   trace.nports = 0;
   trace.nevents = 0;
}

/***************
 * close trace *
 ***************/
extern void hwetrace_close ()
{
	if (trace.nports != 0) {
		for (int i = 0; i < MAXPORTS; i++) {
			if (trace.ports[i] != NULL)
				hwe_port_close(trace.ports[i]);
		}	
	}

   if (trace.name) {
      free(trace.name);
      trace.name = NULL;
   }
}

/*******************
 * start the trace *
 *******************/
static void trace_start()
{
   trace.ports = calloc(MAXPORTS, sizeof(hwe_port_t *));

   if (trace.name)
      handle_start(trace.name, 0 , NULL);
   else
      handle_start(default_name, 0, NULL);

#ifdef HWETRACE_DUMP
#ifdef HWETRACE_DUMPFILE
	trace.dump = fopen(HWETRACE_DUMPFILE, "w");
#else
	trace.dump = stderr;
#endif
#endif
	HWETRACE_PRINT("Starting trace.\n");

	HWETRACE_STAT_PRINT("Statistics enabled\n");
	HWETRACE_TRACESTAT_INIT(&trace.trstat);

#ifdef HWETRACE_PARALLEL
	hwepar_init(&trace.hwepar);
#endif
}

/******************
 * stop the trace *
 ******************/
static void trace_stop()
{
#ifdef HWETRACE_DUMPFILE
	fclose(trace.dump);
#endif

#ifdef HWETRACE_PARALLEL
	hwepar_stop(&trace.hwepar);
#endif

	HWETRACE_PRINT("Stoping trace.\n");
	HWETRACE_TRACESTAT_PRINT("Global tracing ",&trace.trstat);
	HWETRACE_EVENTSTAT_PRINT();

	free(trace.ports);
   handle_stop();
}

/**
 * @brief Create a port
 * @param name
 * @param type
 * @return
 */
static hwe_port_t * add_port (const char *name, hwe_device_t type)
{  
    if (trace.nports == 0)
        trace_start();

    if (trace.nports == MAXPORTS) {
        fprintf(stderr, "HWEtrace: too many opened ports\n");
        return NULL;
    }

    HWETRACE_PRINT("Opening port %u `%s'\n", trace.nports, name);

    hwe_port_t *pt = calloc(1, sizeof(*pt));

    pt->devid   = trace.nports++;
    pt->index   = 0;
    pt->type    = type;
    strncpy(pt->name, name, 32);
    pt->name[31] = '\0';

    trace.ports[pt->devid] = pt;

    HWETRACE_TRACESTAT_INIT(&pt->trstat);

    return pt;
}

/**
  Add a port and generation the info event
 * @param name
 * @param type
 * @param dev
 * @return
 */
hwe_port_t * hwe_port_open (const char *name, hwe_device_t type, 
                               const hwe_devices_u *dev)
{
   /*
    * create port
    */
   hwe_port_t *pt = add_port(name, type);

	/*
    * send INFO event
    */
	if (dev) {
		hwe_cont *cont = hwe_init(pt);
		HWE_INFO_init(cont, type, name, dev);
		hwe_commit(cont);
	}
   
   return pt; 
}

/**
 * @brief Close a port
 * @param port
 */
void hwe_port_close  (hwe_port_t *port)
{
   HWETRACE_PRINT("Closing port %u:'%s'.\n", port->devid, port->name);
	HWETRACE_TRACESTAT_PRINT("Port ",&port->trstat);
   /*
    * remove it from the list
    */
   trace.ports[port->devid] = NULL;

   /*
    * free memory
    */
   free(port);
   
	/*
    * update nports and eventually stop the trace
    */
   if (--trace.nports == 0)
      trace_stop();

}

/**
 * @brief Init an event
 * @param port
 * @return
 */
hwe_cont * hwe_init (hwe_port_t *port)
{ 
   hwe_cont *hwe;
	/*
	 * get a container
	 */
#ifdef HWETRACE_PARALLEL
	hwe = hwepar_alloc(&trace.hwepar);
#else

    hwe = handle_alloc();
    hwe_head_init(&hwe->common);
#endif
#ifdef HWE_SAFEMODE
 	assert(hwe->common.self != HWE_REF_NULL);  
#endif

   /*
    * set up the id
    */
   hwe->common.id.devid = port->devid;
   hwe->common.id.index = port->index++;

   HWETRACE_TRACESTAT_OUT(&trace.trstat,&port->trstat, true);

   return hwe;
}

/***********************
 * Add a ref container *
 ***********************/
hwe_cont * hwe_extend (hwe_cont *hwe)
{
   hwe_cont *ext;
	/*
	 * get a container
	 */
#ifdef HWETRACE_PARALLEL
	ext = hwepar_alloc(&trace.hwepar);
#else
   ext = handle_alloc();
	hwe_head_init(&ext->common);
#endif
 	assert(hwe->common.self != HWE_REF_NULL);  

   /*
    * set same id
    */
	ext->common.id = hwe->common.id;
   
   /*
    * link the new container
    */
	hwe_head_extend(&hwe->common, &ext->common);

	HWETRACE_TRACESTAT_OUT(&trace.trstat, &trace.ports[hwe->common.id.devid]->trstat, false);

   return ext;
}

/**
 * @brief Commit an event
 * @param hwe
 */
void hwe_commit (hwe_cont *hwe)
{
	trace.nevents += 1;
#ifdef HWE_SAFEMODE
	{
		hwe_head_cont *cont = &hwe->common;
		do {
			for (unsigned i = 0; i < cont->head.nrefs; i += 1) {
				if (cont->refs[i].ref == HWE_REF_NULL) {
					hwe_port_t *pt = hwe_get_port(hwe->common.id.devid);
					hwe_print(stderr, hwe, pt?pt->name:"unknown port ???", NULL);
					fprintf(stderr, "Error: Null reference\n");
					exit(EXIT_FAILURE);           
				}
			}
			cont = cont->refnext;
		} while (cont);
	}
#endif

	HWETRACE_TRACESTAT_IN(&trace.trstat, &trace.ports[hwe->common.id.devid]->trstat, &hwe->common);
	HWETRACE_EVENTSTAT_LOG(trace.ports[hwe->common.id.devid]->type, hwe);

#ifdef HWETRACE_DUMP
	{
		static unsigned dump_cnt = 0;
		char cnt[10];
		snprintf(cnt, 10, "%u\n", dump_cnt++);
		cnt[9] = '\0';
		hwe_print(trace.dump, hwe, NULL, cnt, NULL);
	}
#endif

#ifdef HWETRACE_PARALLEL
	hwepar_handle(&trace.hwepar, hwe);
#else
   handle_event(hwe);
#endif
}

/*******************
 * Cancel an event *
 *******************/
void hwe_cancel (hwe_cont *hwe)
{
   // ie: commit an empty event
   hwe->common.head.type = HWE_NULL;
   hwe->common.head.nrefs = 0;
   hwe->common.head.ndates = 0;
   hwe->common.head.expected = 0;
   hwe_commit(hwe);
}

uint64_t hwetrace_event_count()
{
	return trace.nevents;
}

