#include <sherlock.h>
#include <search.h>
#include <debug.h>
#include <util_hash.h>
#include <delorean.h>
#include <core_mem.h>
#include <cfg.h>
//#define DEBUG_TRC_MEMORY 

//#define ALIGN_32BIT       0b11
#define ALIGN_32BIT_SHIFT 2

//static void mem_go_ack(event_t *e);
static void mem_ret_ack(event_t *e);
static void mem_go_access(event_t *e);


//ht_t *mem = NULL;

/**
 *
 * @param e
 *
 * @return void
 */
void mem_init (event_t *e)
{
    struct event_data_t    *ed = event_data(e);

//    struct comp_data_t *cd_ref = comp_data(event_component(event_ref(e,0)));
    struct comp_data_t *cd     = comp_data(event_component(e));

    hwe_cont *hwe = event_content(e);

    switch(hwe->common.head.type) {
    case HWE_MEMACK:
//        if (hwe_rd->mem.body.mem32.access != hwe->ack.body.access ){
//            ERROR("HWE_MEM32 request and HWE_MEMACK acknowledge have different kind of access");
//            exit(1);
//        }
        // Verify if the last operation was a write -> came from either Cache, either Processor, or device
        set_cb_go (e, cb_nop);
        set_cb_ret(e, mem_ret_ack);
        put_infifo_ret(&cd->fifo, e);
        // TODO:put the switch case here 
        break;
    case HWE_MEM32: // PERIPHERAL // TODO: It should not exist anymore
        ERROR("Such event should not be produced by memories anymore"); 
        set_cb_go (e, mem_go_access);
        set_cb_ret(e, cb_nop);
        ed->type = EVENT_DEVICE_FILL;
        put_infifo_ret(&cd->fifo, e);
        exit(1);
        break;
    default:
        set_cb_go (e, cb_nop);
        set_cb_ret(e, cb_nop);
        ERROR("Memory event not defined");
        exit(1);
        break;
    }
}

/**
 *
 * @param e
 *
 * @return void
 */
/*static void mem_go_ack(event_t *e)
{
    printf("[%d.%d] GO\n", (uint32_t)e->hwe->common.id.devid, (uint32_t)e->hwe->common.id.index);
}*/


/**
 *
 * @param e
 *
 * @return void
 */
static void mem_ret_ack(event_t *e)
{
//    hwe_cont *hwe = event_content(e);
    hwe_cont *hwe_rd = event_content(event_ref(e,0));
    struct comp_data_t  *cd = comp_data(event_component(e));

//    struct event_data *ed = event_data(e);
    struct event_data_t *rd = event_data(event_ref(e,0));
//    struct comp_data_t  *cd_ref = comp_data(event_component(event_ref(e,0)));

    hwe_mem_t access;
    uint32_t addr = hwe_rd->mem.body.mem32.addr;

    gdb_param_t gdb_param;
    
//    if (cd_ref->comp.cache != NULL){
//        if( cd_ref->comp.cache->comp_type == COMP_CACHE){
//            if (hwe_rd->mem.body.mem32.access != hwe->ack.body.access ){
//                ERROR("HWE_MEM32 request and HWE_MEMACK acknowledge have different type of access");
//                exit(1);  
//            }
//        }
//    }

    switch(hwe_rd->mem.body.mem32.access){
    case HWE_MEM_LOAD:
    case HWE_MEM_PREF:
        access = EVENT_ACK_READ;
        break;
    case HWE_MEM_STORE:
        access = EVENT_ACK_WRITE;
        break;
    default:
        access= EVENT_NOOP;
        break;
    }

    if (rd->type == EVENT_FILL){
        int width;
        uint8_t *data;
        item_t* item = NULL;

//        PEVENT(e->hwe);
        width = (hwe_rd->mem.body.mem32.width + 1); // +1 necessary due trace formation
        data  = hwe_rd->mem.data;

        item = mem_exec(cd->comp.mem, addr, data, width);
#ifdef DEBUG_TRC_MEMORY        
        if( addr % 4){
            printf("@0x%08x => A@0x%08x = 0x%08x\n", addr
                    , addr & ~0x3
                    , item->data->value);
        } else { 
            printf("@0x%08x = 0x%08x\n", addr
                    , item->data->value); 
        }
#endif
        pg_exec_mem(cd->comp.mem, item->data, rd->inst_node, access, addr, e);

        gdb_param.mem.id   = cd->comp.mem->id;
#ifdef CPU_METADATA
        if(rd->inst_node != NULL)
            gdb_param.mem.ref  = rd->inst_node->data.inst->id;
#endif
        gdb_param.mem.addr = addr;
        gdb_param.mem.val  = item->data->value;

        gdb_verify(COMP_MEM, &gdb_param);

/*    } else if (rd->type == EVENT_DEVICE_FILL){ // TODO: Must be removed!!!!
        int width;
        int addr;
        uint8_t *data;
        item_t* item = NULL;

        ERROR("It should not be pass here anymore: must be migrated to core_periph.c");
        width = (hwe_rd->mem.body.mem32.width + 1); // +1 necessary due trace formation
/        addr  = hwe_rd->mem.body.mem32.addr;
        data  = hwe_rd->mem.data;

        item = mem_exec(cd->comp.mem, addr, data, width);
#ifdef DEBUG_TRC_MEMORY
        printf("DEVICE WRITE\t");

        if( addr % 4){
            printf("@0x%08x => A@0x%08x = 0x%08x\n", addr
                    , addr & ~0x3
                    , item->data->value);
        } else {
            printf("@0x%08x = 0x%08x\n", addr
                    , item->data->value);
        }
#endif
        pg_exec_mem(cd->comp.mem, item->data, rd->inst_node, access, addr, e);

        gdb_param.mem.id   = cd->comp.mem->id;
#ifdef CPU_METADATA
        if(rd->inst_node != NULL){
            gdb_param.mem.ref  = rd->inst_node->data.inst->id;
        }
#endif
        gdb_param.mem.addr = addr;
        gdb_param.mem.val  = item->data->value;

        gdb_verify(COMP_MEM, &gdb_param);*/
    }else {
        pg_exec_mem(NULL, NULL, NULL, access, addr, e);
    }
}

/**
 * @brief 
 *
 * @param e
 */
void mem_go_access(event_t *e)
{  
    ERROR("AQUI");
    struct comp_data_t *cd = comp_data(event_component(e));
     
    pg_exec_dev(cd->comp.mem);

    e->data.inst_node = oracle_get_last_exec();
}


//==========================================
/**
 * @brief 
 *
 * @param e
 *
 * @return 
 */
mem_t* mem_create(event_t *e)
{
    FILE *fd;
    uint32_t sec, sec_cur, sec_size, ofs, i, j;
    item_t *item;
    mem_t *mem_dev;
    
    hwe_cont *hwe = event_content(e);

    mem_dev = malloc(sizeof(mem_t));

    // Set ID
    mem_dev->id =  hwe->common.id.devid;

    // Fill the first values, ( program binary - specially)
    // Read a File with values 
    fd = fopen("mem_init", "r");

    if (fd == NULL){
        perror("mem_init: ");
        exit(1);
    }

    fscanf(fd, "N_SECTIONS=%08x\n", &sec);
#ifdef DEBUG_INIT_MEM
    printf("%s: TOTAL SECTIONS %d\n", __func__ , sec);
#endif // DEBUG_INIT_MEM

    for(i = 0; i < sec; i++){
//        uint32_t* mem;
        uint8_t* mem;

        if(fscanf(fd, "SECTION=%08x\nSIZE=%08x\nOFFSET=%08x\n", &sec_cur,&sec_size,&ofs) != 3){
            printf("Arguments SECTION/SIZE/OFFSET/ did not find @ %ld\n",ftell(fd));
            fclose(fd);
            exit(1);
        }

//#ifdef DEBUG_INIT_MEM
        printf("%s: SECTION %x\n", __func__, sec_cur);
        printf("%s: SIZE    0x%08x\n",  __func__, sec_size);
        printf("%s: OFFSET  %x\n", __func__, ofs);
//#endif // DEBUG_INIT_MEM

//        mem = (uint32_t*)malloc(sizeof(uint8_t)*sec_size);
        mem = (uint8_t*)malloc(sizeof(uint8_t)*sec_size);


        fread(mem, sec_size, sizeof(uint8_t),fd);
       
//        for(j = 0; j < (sec_size>>2); j++){
          for(j = 0; j < (sec_size); j++){
//            if (mem[j]){ // 32bits aligned
            if (mem[j]){ // 8bits aligned
                // Judy Structure initialization
                item = (item_t*)ht_get(ofs + j);
                // Fill item with parameters
                item->data->value = mem[j];
            }
        }
        free(mem);
    }
    return mem_dev;
}


/**
 * @brief 
 */
void mem_destroy(void)
{
   // FOR EVERY MEMORY
//    ht_free(mem);
}

/**
 * @brief 
 *
 * @param mem
 * @param addr
 * @param data
 * @param width
 *
 * @return 
 */
void *mem_exec(mem_t *mem, uint32_t addr, uint8_t *data, uint32_t width)
{
    item_t  *item = NULL;
    uint32_t addr_key = addr >> ALIGN_32BIT_SHIFT;

    item = ht_get(addr_key);

    if(!(addr & ALIGN_32BIT)){ // for word aligned accesses
        uint32_t mask;
        switch (width){
        case 1:
            mask = ~0xFF;
            break;
        case 2:
            mask = ~0xFFFF;
            break;
        case 3:
            mask = ~0xFFFFFF;
            break;
        default:
            mask = 0x0;
            break;
        }
        if(addr == 0x0001ee74){
           __asm__("nop");
        }

        item->data->value =  (item->data->value & mask) | ((*(uint32_t*)data) & ~mask );


    } else { // for byte unaligned accesses
        uint32_t tmp, byte, mask;
        uint32_t *item_data = &item->data->value;  
        byte = addr & ALIGN_32BIT;

        switch (width){
        case 1:
            mask = ~(0xFF << (byte << 3));
            break;
        case 2:
            mask = ~(0xFFFF << (byte << 3));
            break;
        case 3:
            mask = ~(0xFFFFFF << (byte << 2));
            break;
        default:
            mask = 0x00000000;
            ERROR("32 bits aligned access, assuming 4 bytes");
            break;
        }
        tmp  = (((*(uint32_t*)data ) << (byte << 3))) & ~mask;
        *item_data &= mask;
        *item_data |= tmp;
    }
    return item;
}

/**
 * @brief 
 *
 * @param addr
 *
 * @return 
 */
uint32_t mem_get_addr_value(uint32_t addr)
{
//    item_t* item =  ht_get(addr >> 2);
    item_t* item =  ht_get(addr);
    return item->data->value;
}

/**
 * @brief 
 *
 * @param addr
 *
 * @return 
 */
sl_addr_t *mem_get_addr(uint32_t addr)
{
//    item_t* item =  ht_get(addr >> 2);
    item_t* item =  ht_get(addr);
    return item->data;
}
