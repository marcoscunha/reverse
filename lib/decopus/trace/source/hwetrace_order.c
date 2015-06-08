
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
//#define HANDLE_VERBOSE 1
#include <hwe_handle_def.h>

struct comp_data_t {
    evfifo_t fifo;    // fifo
};
static evfifo_t instfifo;

#define USE_STAGE_GO
#define USE_STAGE_RET
#define COMP_DATA_T struct comp_data_t
//#define EVENT_DATA_T
#include <hwe_handle_header.h>

#include "hwe_handle_main.h"

#define THRESHOLD 1000
//#define  DEBUG_ORDER

typedef struct{
   uint32_t dev; 
   uint32_t current;
}order_t;

order_t *order = NULL;

/*
 * init
 */
void process_init(const char *tracename, int nopt, char * const opt[])
{
// Create arguments processor structures
    evfifo_init(&instfifo);
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
static void cb_go(event_t *e){
    hwe_cont *hwe = event_content(e);

    if ( (order[hwe->common.id.devid].current + 1) != hwe->common.id.index ){
        printf("GO  Events are out of order [%d.%d]->[%d.%d] [%d]\n",(uint32_t)hwe->common.id.devid,
                                                                order[hwe->common.id.devid].current,
                                                                (uint32_t)hwe->common.id.devid,
                                                                (uint32_t)hwe->common.id.index,
                                                                (uint32_t)hwe->common.head.type);

//        exit(1);
    }
#ifdef DEBUG_ORDER
    else {
        printf("GO  Events are ordered      [%d.%d]->[%d.%d]\n",(uint32_t)hwe->common.id.devid,
                                                                order[hwe->common.id.devid].current,
                                                                (uint32_t)hwe->common.id.devid,
                                                                (uint32_t)hwe->common.id.index);
    }
#endif    
   order[hwe->common.id.devid].current = hwe->common.id.index;
}

static void cb_ret(event_t *e){
    hwe_cont *hwe = event_content(e);

    if( (order[hwe->common.id.devid].current + 1) != hwe->common.id.index ){
        printf("RET Events out of order      [%d.%d]->[%d.%d] [%d]\n",(uint32_t)hwe->common.id.devid,
                                                                order[hwe->common.id.devid].current,
                                                                (uint32_t)hwe->common.id.devid,
                                                                (uint32_t)hwe->common.id.index,
                                                                (uint32_t)hwe->common.head.type);

//      exit(1);
    }
#ifdef DEBUG_ORDER    
  else {
        printf("RET Events are ordered      [%d.%d]->[%d.%d]\n",(uint32_t)hwe->common.id.devid,
                                                                order[hwe->common.id.devid].current,
                                                                (uint32_t)hwe->common.id.devid,
                                                                (uint32_t)hwe->common.id.index);
   }
#endif
   order[hwe->common.id.devid].current = hwe->common.id.index;
}


static void cb_init_cpu(event_t *e) {
    hwe_cont *hwe = event_content(e);
    struct comp_data_t *cd = comp_data(event_component(e));


//    set_cb_go(e, cb_go);
//    set_cb_ret(e, cb_ret);

//    printf("[%d.%d]\n",(uint32_t)e->hwe->common.id.devid,(uint32_t) e->hwe->common.id.index);

#if HWE_HANDLE_REPORT_LVL >= 4
   if (e->comp->rep_queue &&
         e->comp->rep_queue->hwe->common.id.index + THRESHOLD <
         e->hwe->common.id.index) {
      fprintf(stderr, "Warning: got %u.%"HWE_PRI_ID" but %u.%"HWE_PRI_ID" still here\n",
            e->hwe->common.id.devid, e->hwe->common.id.index,
            e->comp->rep_queue->hwe->common.id.devid, e->comp->rep_queue->hwe->common.id.index);
      handle_report();
      exit(1);
   }
#endif
    switch(hwe->common.head.type){
    case HWE_INST32:
        put_infifo_go(&cd->fifo, e);
        set_cb_go(e, cb_go);
        set_cb_ret(e, cb_nop);
    break;
    default:
        put_infifo_go(&cd->fifo, e);
        set_cb_go(e, cb_go);
        set_cb_ret(e, cb_nop);
    break;
    }
}

static void cb_init_comp(event_t *e) {
    hwe_cont *hwe = event_content(e);
    struct comp_data_t *cd = comp_data(event_component(e));


//    set_cb_go(e, cb_go);
//    set_cb_ret(e, cb_ret);

// printf("[%d.%d]\n",(uint32_t)e->hwe->common.id.devid,(uint32_t) e->hwe->common.id.index);

#if HWE_HANDLE_REPORT_LVL >= 4
   if (e->comp->rep_queue &&
         e->comp->rep_queue->hwe->common.id.index + THRESHOLD <
         e->hwe->common.id.index) {
      fprintf(stderr, "Warning: got %u.%"HWE_PRI_ID" but %u.%"HWE_PRI_ID" still here\n",
            e->hwe->common.id.devid, e->hwe->common.id.index,
            e->comp->rep_queue->hwe->common.id.devid, e->comp->rep_queue->hwe->common.id.index);
      handle_report();
      exit(1);
   }
#endif
    switch(hwe->common.head.type){
    case HWE_MEMACK:
        if(hwe->common.id.devid > 0){
            put_infifo_ret(&cd->fifo, e);
            set_cb_go(e, cb_nop);
            set_cb_ret(e, cb_ret);
        }else{ // MEMORY
            put_infifo_ret(&cd->fifo, e);
//            set_cb_go(e, cb_nop);
//          set_cb_ret(e, cb_ret);
            set_cb_go(e, cb_nop);
            set_cb_ret(e, cb_ret);
        }
    break;
    default:
      put_infifo_ret(&cd->fifo, e);
      set_cb_go(e, cb_nop);
      set_cb_ret(e, cb_ret);
    break;
    }
}

void process_component(comp_t *comp, event_t *e)
{
   static uint32_t n_comp = 0;
   hwe_cont *hwe = event_content(e);

   n_comp++;
   order = realloc(order, sizeof(order_t)*n_comp);
   order[n_comp-1].dev = hwe->common.id.devid;
   order->current = 0;

   if (hwe->common.id.devid != order[hwe->common.id.devid].dev){
      printf("INFO Events are out of order for %d -> [%d]!=[%d]\n",(uint32_t)hwe->common.id.devid,
            hwe->common.id.devid, order[hwe->common.id.devid].dev);
      exit(1);
   }else{
      printf("INFO Events are orderes for %d -> [%d]==[%d]\n",(uint32_t)hwe->common.id.devid,
            hwe->common.id.devid, order[hwe->common.id.devid].dev);
   }
   while(hwe){
       hwe_info_cont *info = &hwe->info;
       switch(info->body.device){
       case HWE_PROCESSOR:
           set_cb_init(comp, cb_init_cpu);
           break;
       case HWE_CACHE:
       case HWE_MEMORY:
           set_cb_init(comp, cb_init_comp);
           break;
       }
       hwe = (hwe_cont *) hwe->common.refnext;
   }

}

