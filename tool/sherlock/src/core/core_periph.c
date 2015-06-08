#include <sherlock.h>
#include <debug.h>
#include <core_periph.h>

static void periph_go_req(event_t *e);

/**
 *
 * @param e
 *
 * @return void
 */
void periph_init (event_t *e)
{
    //struct event_data_t *ed = event_data(e);
    struct comp_data_t *cd = comp_data(event_component(e));

    hwe_cont *hwe = event_content(e);
    switch(hwe->common.head.type) {
    case HWE_MEM32:
        put_infifo_ret(&cd->fifo, e);
        set_cb_go (e, periph_go_req);
        set_cb_ret(e, cb_nop);
        break;
    default:
        ERROR("Event not defined");
        printf("\t Event type = %d\n", hwe->common.head.type);
//        exit(1);
        break;
    }

}

static void periph_go_req(event_t *e)
{
}



