
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "events/hwe_tools.h"

/***************************************************
 * 'hwe handler' which does nothing but some check *
 ***************************************************/

#define HWE_HANDLE_REPORT_LVL 4

#define USE_STAGE_GO
#define USE_STAGE_RET
//#define COMP_DATA_T struct comp_data_t
//#define EVENT_DATA_T
#include "hwe_handle_main.h"

/*
 * init
 */
void process_init(const char *tracename, int nopt, char * const opt[])
{
   ;
}

/*
 * stop
 */
void process_stop()
{
   ;
}

/*
 * start
 */
void process_start()
{
   ;
}

/*
 * info
 */
static void cb_nop(event_t *e) {
   ;
}
static void cb_init(event_t *e) {
   set_cb_go(e, cb_nop);
   set_cb_ret(e, cb_nop);

//   printf("[%d.%d]\n",(uint32_t)e->hwe->common.id.devid,(uint32_t) e->hwe->common.id.index);
#if HWE_HANDLE_REPORT_LVL >= 4
   if (e->comp->rep_queue && 
         e->comp->rep_queue->hwe->common.id.index + 800 <
         e->hwe->common.id.index) {
      fprintf(stderr, "Warning: got %u.%"HWE_PRI_ID" but %u.%"HWE_PRI_ID" still here\n",
            e->hwe->common.id.devid, e->hwe->common.id.index,
            e->comp->rep_queue->hwe->common.id.devid, e->comp->rep_queue->hwe->common.id.index);
      handle_report();
      exit(1);
   }

#endif
}
void process_component(comp_t *comp, event_t *e)
{
   set_cb_init(comp, cb_init);
}

