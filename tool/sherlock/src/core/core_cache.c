#include <sherlock.h>
#include <debug.h>
#include <core_cache.h>

static void cache_go_req (event_t *e);
static void cache_ret_req(event_t *e);
//static void cache_go_ack(event_t *e);
static void cache_ret_ack(event_t *e);

/**
 *
 * @param e
 *
 * @return void
 */
void cache_init (event_t *e)
{
  struct event_data_t *ed = event_data(e);
  struct comp_data_t *cd = comp_data(event_component(e));
    hwe_cont *hwe = event_content(e);

    switch(hwe->common.head.type)
    {
    case HWE_MEMACK:
        set_cb_go (e, cb_nop);
        set_cb_ret(e, cache_ret_ack);
        put_infifo_ret(&cd->fifo, e);
        break;
    case HWE_MEM32:
         switch(hwe->mem.body.mem32.access) {
         case HWE_MEM_LOAD:
             set_cb_go (e, cache_go_req);
             set_cb_ret(e, cache_ret_req);
             put_infifo_ret(&cd->fifo, e);
             break;
         case HWE_MEM_STORE:
             set_cb_go (e, cache_go_req);
             set_cb_ret(e, cache_ret_req); 
             ed->type = EVENT_FILL;
             put_infifo_ret(&cd->fifo, e);
             break;
         case HWE_MEM_PREF:
             set_cb_go (e, cb_nop);
             set_cb_ret(e, cb_nop);
             put_infifo_ret(&cd->fifo, e);
             break;
         default:
             set_cb_go (e, cb_nop);
             set_cb_ret(e, cb_nop);
             printf("%d\n",hwe->mem.body.mem32.access );
             ERROR("Event HWE_MEM32 not defined");
             exit(1);
             break;
         }
         break;
    default:
        ERROR("Event not defined");
        exit(1);
        break;
    }

}

/**
 * @brief 
 *
 * @param e
 */
/*static void cache_ret_req(event_t *e)
{
    printf("[%d.%d] RET\n", (uint32_t)e->hwe->common.id.devid, (uint32_t)e->hwe->common.id.index);
}*/

/**
 *
 * @param e
 *
 * @return void
 */
static void cache_go_req(event_t *e)
{
    struct event_data_t *rd = event_data(event_ref(e,0));
    struct event_data_t *ed = event_data(e);
    
    ed->inst_node = rd->inst_node;

//  printf("[%d.%d] GO\n", (uint32_t)e->hwe->common.id.devid, (uint32_t)e->hwe->common.id.index);

}

/**
 * @brief 
 *
 * @param e
 */
static void cache_ret_req(event_t *e)
{
//    struct event_data_t *ed = event_data(e);
    struct comp_data_t *cd = comp_data(event_component(e));
    hwe_cont *hwe = event_content(e);
    uint8_t access;
 
    switch(hwe->mem.body.mem32.access){
    case HWE_MEM_LOAD:
        access = EVENT_READ;
        break;
    case HWE_MEM_STORE:
        access = EVENT_WRITE;
        break;
    default:
        access = EVENT_NOOP;
        ERROR("Event not valid.");
        break;
    }    
//    if(hwe->mem.body.mem32.addr == 0x0002486c){
//        PEVENT(hwe); printf("REQ addr 0x%08x\n", hwe->mem.body.mem32.addr);
//}
    pg_exec_cache(cd->comp.cache, access, hwe->mem.body.mem32.addr,e);

}

/**
 *
 * @param e
 *
 * @return void
 */
/*static void cache_go_ack(event_t *e)
{

}*/

/**
 * @param e
 *
 * @return void 
 */
static void cache_ret_ack(event_t *e)
{
//    struct event_data_t *ed = event_data(e);
    struct comp_data_t *cd = comp_data(event_component(e));

    hwe_cont *hwe = event_content(e);
    uint8_t access;
    
    switch(hwe->mem.body.mem32.access){
    case HWE_MEM_LOAD:
        access = EVENT_ACK_READ; 
        break;
    case HWE_MEM_STORE:
        access = EVENT_ACK_WRITE;
        ERROR("Valid just for writeback methods, update: even not valid in this case");
        exit(1);
        break;
    case  HWE_MEM_MODIFY:
        access = EVENT_ACK_MODIFY;
        break;
    case HWE_MEM_PREF:
        access = EVENT_NOOP;
        break;
    default:
        access = EVENT_NOOP;
        ERROR("Event not valid.");
        exit(1);
        break;
    }
//    if ( hwe->ack.body.addr == 0x0002486c){
//        PEVENT(hwe); printf("ACK addr 0x%08x\n", hwe->ack.body.addr);
//}
    pg_exec_cache(cd->comp.cache, access, hwe->ack.body.addr, e);

}

inline cache_t*  cache_create(event_t *e)
{
    hwe_cont* hwe = event_content(e);
    cache_t* cache;
    cache = calloc(1, sizeof(cache_t));

    cache->id = hwe->common.id.devid;
    cache->type = strncmp(hwe->info.name, "I", 1) ? DCACHE : ICACHE;
    cache->comp_type = COMP_CACHE;

    return cache;

}

void cache_destroy(cache_t* cache)
{
    free(cache);
}
