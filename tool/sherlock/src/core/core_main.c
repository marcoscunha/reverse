#include <sherlock.h>
#include <debug.h>
#include <plugin.h>
#include <core_cpu.h>
#include <core_cache.h>
#include <core_mem.h>
#include <core_periph.h>

#include <hwe_handle_main.h>


static evfifo_t instfifo;

struct event_data_go_t{
//   int cpuid;    //cpuid of initial request
//   event_t *src; //initial request event
};

struct event_data_ret_t {
   // data transfered during RET propagation
//   comp_t *periph; //peripheral which received the request
};


//static checked_devices_t devices;

/**
 * @brief
 * 
 * @param tracename
 * @param narg
 * @param arg
 *
 * @return void
 */
void process_init(const char *tracename, int narg, char *const arg[])
{
    // Put the arguments analysis 
    printf("process_init\n");
    
    // plugin init
   
    pg_init(narg, (char**)arg);
    
    // Create arguments processor structures
    evfifo_init(&instfifo);
}

/*
 *  * start
 *   */
void process_start()
{

}

/**
 * @brief
 * 
 * @param c
 * @param e
 *
 * @return void 
 */
void process_component(comp_t *c, event_t *e)
{
    hwe_cont *hwe = event_content(e);
    struct comp_data_t *cd = comp_data(c);

    while(hwe){
        hwe_info_cont *info = &hwe->info;
 
        switch(info->body.device){
        case HWE_PROCESSOR:           
           printf("HWE_PROCESSOR [%d] - Init\n", hwe->common.id.devid);
           // Init processor
           cd->comp.cpu = cpu_create(e); // Init registers
           // Init Regs
           // Oracle must be initialized here!
           pg_comp(cd->comp.cpu, NULL, COMP_CPU);
           set_cb_init(c, cpu_init); // Init event handler
           break;
        case HWE_CACHE:
           printf("HWE_CACHE [%d] - Init\n", hwe->common.id.devid);
           // Init cache structures
           cd->comp.cache = cache_create(e);
           cd->comp.cache->id = hwe->common.id.devid;
           pg_comp(cd->comp.cache, NULL, COMP_CACHE);
           set_cb_init(c, cache_init);
           break;
        case HWE_MEMORY:
           printf("HWE_MEMORY [%d] - Init\n", hwe->common.id.devid);
           cd->comp.mem = mem_create(e); 
           pg_comp(cd->comp.mem, NULL,COMP_MEM);
           // Create memory structures
           set_cb_init(c, mem_init);
           break;
        case HWE_PERIPHERAL:
           printf("HWE_PERIPHERAL [%d] - Init\n", hwe->common.id.devid);
           pg_comp(cd->comp.mem, NULL,COMP_PERIPH);
           cd->comp.cpu = NULL;
           // periph_create(e);
           set_cb_init(c, periph_init);
           break;
        default:
           printf("NOT DEFINED [%d] - Init\n", hwe->common.id.devid);
           exit(1);
           break;
        }
        hwe = (hwe_cont *) hwe->common.refnext;
    }
}

/**
 * @brief 
 * 
 * @return void
 */
void process_stop(void)
{
    printf("Last chance to analyse!\n");
    pg_exit();
    mem_destroy();
}

/******************
 * MAIN CALLBACKS *
 ******************/
/**
 * @brief 
 *
 * @param e
 * @return void
 */
void cb_nop(event_t *e)
{

}  

/*
 * EXECUTION FUNCTIONS 
 */
// update registers and memory when the case occurs 
//void core_exec(sl_state_t *state)
//{

//   hwe_cont *hwe = event_content(e);
   // Update the values and the plugin modify the metadata 


   // TODO: call the plugin function to update metadata
 

   // if it is a jumper
   // if it is executed
   // if it is aligned 
   // if it is 
   
   
// switch(){

// }

//  pg_inst(e);


//}

//int main (int argc, char** argv)
//{

 /* Read the architecture files
    - Make this one as a simple XML file 
    - Create memory structure
    - Based on number of processors:
        * Create the registers memory
        * Create cache structures
    - Create basic structures for metadata
    - Add/remove 
    - 
  Identify and read the trace files 
    char *tool = "analysis"; 
    Verifing tool
   Stating */

//    for(){
  //   core_reg_alloc();
//    }
//  core_reg_alloc();

//  pg_exec();

//    pg_exit();
/*    sl_post_args();
    Starting services demanded by tool

   Finishing the execution   

   sl_end  */

//}






