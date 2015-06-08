/*
 *  Copyright (c) 2010 TIMA Laboratory
 *
 *  This file is part of Rabbits.
 *
 *  Rabbits is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Rabbits is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Rabbits.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <cfg.h>
#ifdef TRACE_EVENT_ENABLED
#include <hwetrace.h>
#include <hwetrace_api.h>
#include <events/hwe_device.h>
#include <hwetrace_memory.h>
#include <hwetrace_cache.h>
#include <unordered_map>

#include "../trace_port/trace_port.h"
#endif

#include <mem_device.h>

using namespace std;

#define CACHE_MASK 0xFFFFFFE0
#define HASHSIZE 256

uint8_t enable_trace = false;

//#define DEBUG_TRACE_MEM_READ
//#define DEBUG_MEM

#ifdef DEBUG_MEM
#define DPRINTF(fmt, args...)                   \
    do { printf(fmt , ##args); } while (0)
#define DCOUT if (1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if (0) cout
#endif

//#define COMMIT_DEBUG
#ifdef COMMIT_DEBUG
bool dflag = false;
#define REF_DEVICE 2
#define START_REF_INDEX 0 

#define SAFE_TEST m_req.hwe_cont_ref != 0
//#define COND  (((hwe_cont*)m_req.hwe_cont_ref)->common.id.index > START_REF_INDEX && ((hwe_cont*)m_req.hwe_cont_ref)->common.id.devid == REF_DEVICE) 
#define COND 1  
#define DCOMMIT(hwe) if(SAFE_TEST)\
if(COND || dflag){dflag= true; \
cout <<dec<< left << setw(10) << "COMMIT" << "["<<(uint32_t)hwe->common.id.devid << "." << left << setw(15) << (uint32_t)hwe->common.id.index << "]"; \
if (((hwe_cont*)m_req.hwe_cont_ref) != 0 ){\
      hwe_cont* hwe_ref;\
      if(((hwe_cont*)m_req.hwe_cont_ref)->common.id.devid != 0 &&\
         ((hwe_cont*)m_req.hwe_cont_ref)->common.id.index != 0){\
          hwe_ref = (hwe_cont*)m_req.hwe_cont_ref;\
      }else{\
          hwe_ref = HWE_HEAD_get_parent((hwe_cont*)m_req.hwe_cont_ref);\
      }\
      cout << "<-["<<(uint32_t)hwe_ref->common.id.devid << "." << left << setw(15) << (uint32_t)hwe_ref->common.id.index << "] ";\
}else{\
    cout << setw(22) << "";}\
cout <<__func__ << ":" << __LINE__ <<endl;}
#else
#define DCOMMIT(hwe)
#endif

#ifdef TRACE_EVENT_ENABLED
#define MEM_INIT_CONSTRUCT  1
#define MEM_INIT_SECTION    2 
#define MEM_INIT_APPEND     3
#define MEM_INIT_CLOSE      4
#define MEM_INIT_DESTROY    5
#endif

template < typename T >
inline T highbit(T& t)
{
    return t = (((T)(-1)) >> 1) + 1;
}


template < typename T >
std::ostream& bin(T& value, std::ostream &o)
{
    for ( T bit = highbit(bit); bit; bit >>= 1 )
    { 
        o << ( ( value & bit ) ? '1' : '0' );
    }
    return o;
}

void mem_device::dump(void)
{
    for( uint32_t i = 0; i < (size>>2); i++){
        if(mem[i]){
            printf("@%08x = 0x%08x\n", i<<2, mem[i]);
        }
    }
}

/**
 * @brief 
 *
 * @param _name
 * @param _size
 * @param sub
 */
mem_device::mem_device (const char *_name, unsigned long _size, generic_subsystem *sub) :
slave_device (_name)
{

    m_subsys = sub;
    m_write_invalidate = true;
    mem = NULL;
    size = _size;
    mem = new unsigned char [size];
    memset (mem, 0, size);

#ifdef TRACE_EVENT_ENABLED
    hwe_device_t type = HWE_MEMORY;
    hwe_devices_u dev;

    dev.memory.baseaddr = 0x0; // TODO: It is not so correct and it must be configurable.
    dev.memory.endaddr  = size-1;
    dev.memory.cached   = 1;

    // TRACE Library
    // MEM Device
    hwe_mem_port = hwe_port_open ("MEMORY", type, &dev);

    // External device - except CPUs 
    type = HWE_PERIPHERAL;
    memset(&dev, 0, sizeof(hwe_devices_u));
   
    hwe_periph_port = hwe_port_open ("PERIPHERAL", type, &dev);

#endif
    DPRINTF ("mem_device: Memory area location: 0x%08x\n", *mem);
}

/**
 * @brief 
 */
mem_device::~mem_device ()
{
    if (mem)
        delete [] mem;
}

/**
 *
 * @param ofs
 * @param be
 * @param data
 * @param bErr
 */
void mem_device::write (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr)
{
    int                 offset_dd = 0;
    int                 mod = 0;
    int                 err = 0;

    bErr = false;
    wait (1, SC_NS);

    if (ofs > size || be == 0)
        err = 1;

    if (!err)
    {
        switch (be)
        {
            //byte access
        case 0x01: offset_dd = 0; mod = 1; break;
        case 0x02: offset_dd = 1; mod = 1; break;
        case 0x04: offset_dd = 2; mod = 1; break;
        case 0x08: offset_dd = 3; mod = 1; break;
        case 0x10: offset_dd = 4; mod = 1; break;
        case 0x20: offset_dd = 5; mod = 1; break;
        case 0x40: offset_dd = 6; mod = 1; break;
        case 0x80: offset_dd = 7; mod = 1; break;
            //word access
        case 0x03: offset_dd = 0; mod = 2; break;
        case 0x0C: offset_dd = 1; mod = 2; break;
        case 0x30: offset_dd = 2; mod = 2; break;
        case 0xC0: offset_dd = 3; mod = 2; break;
            //dword access
        case 0x0F: offset_dd = 0; mod = 4; break;
        case 0xF0: offset_dd = 1; mod = 4; break;
            //qword access
        case 0xFF: offset_dd = 0; mod = 8; break;

        default:
        {
            uint8_t       tbe = be;
            while ((tbe & 1) == 0)
            {
                tbe >>= 1;
                offset_dd++;
            }
            mod = offset_dd;
            while (tbe & 1)
            {
                *((uint8_t*) (mem + ofs) + offset_dd) =
                    *((uint8_t*) data + offset_dd);
                tbe >>= 1;
                offset_dd++;
            }
            mod = offset_dd - mod;
            if ((mod != 1 && mod != 2 && mod != 4 && mod != 8) || tbe)
                err = 1;
            else
                err = 2;
        }
        }

        if (!err){
            switch (mod)
            {
            case 1:
                *((uint8_t*) (mem + ofs) + offset_dd) =
                    *((uint8_t*) data + offset_dd);
                break;
            case 2:
                *((uint16_t*) (mem + ofs) + offset_dd) =
                    *((uint16_t*) data + offset_dd);
                break;
            case 4:
                *((uint32_t*) (mem + ofs) + offset_dd) =
                    *((uint32_t*) data + offset_dd);
                break;
            case 8:
                printf("DWord Access\n"); 

                *((uint32_t*) (mem + ofs) + 0) = *((uint32_t*) data + 0);
                *((uint32_t*) (mem + ofs) + 1) = *((uint32_t*) data + 1);
                printf("DESIRED @0x%08x\t= 0x%08x\n", ofs + 0
                                                    , *((uint32_t*)data + 0));
                printf("DESIRED @0x%08x\t= 0x%08x\n", ofs + 4
                                                    , *((uint32_t*)data + 1));
                break;
            default:
                err = 1;
            }
        }
    }
 #ifdef TRACE_EVENT_ENABLED
    // Mark the modified address
    if(!enable_trace){ // Use a bitmap to verify modified adresses
        analyse_mem_mod(ofs  + (offset_dd * mod));
    } else { // After trace enabling start monitoring the shared variables
        analyse_mem_shared(ofs  + (offset_dd * mod), MEM_WRITE);
    }

    uintptr_t ptr = (uintptr_t)((uint8_t*)(mem+ofs)+(offset_dd*mod));
   
    if (m_req.hwe_cont_ref != 0 ){
#ifdef RABBITS_TRACE_EVENT_CACHE
        HWE_CACHE_mem_set_data((hwe_cont*)m_req.hwe_cont_ref,(uint8_t*)ptr);
#endif
    }/*else if(enable_trace){
        // Create a fake request 
        hwe_cont *req = hwe_init(hwe_periph_port);
        HWE_MEM_mem_init(req, 0, HWE_MEM_STORE, (ofs+(offset_dd*mod)), mod); 
        HWE_HEAD_set_date(req, 0, sc_time_stamp().value()); // begin date
        HWE_HEAD_set_date(req, 1, sc_time_stamp().value()); // end   data
        HWE_MEM32_set_data(req,(uint8_t*)ptr);
        HWE_MEM32_set_rcvack(req); // XXX: usefulless
       
        // Create a real ack!
        hwe_cont *ack = hwe_init(hwe_mem_port);
        HWE_MEM_ack_init(ack); 
        HWE_HEAD_set_child(req, ack);
        HWE_HEAD_set_ref(ack, 0, HWE_HEAD_this(req));
        HWE_MEM_ack_set_date(ack, sc_time_stamp().value());

        DCOMMIT(ack);
        hwe_commit(ack);
        DCOMMIT(req);
        hwe_commit(req);
#ifdef DEBUG_TRACE_MEM
        printf("DEVICE WRITE\t");
#endif
    }*/
#ifdef DEBUG_TRACE_MEM
    uintptr_t ptr_aligned = ptr &  ~0x3;
    if ((ofs+(offset_dd*mod))%4){ 
        printf("@0x%08x => A@0x%08x = 0x%08x\n", (ofs+(offset_dd*mod))
                                               , (ofs+(offset_dd*mod))&(~0x3)
                                               , *(uint32_t*)ptr_aligned);
    } else {
        printf("@0x%08x = 0x%08x\n", (ofs+(offset_dd*mod))
                                   , *(uint32_t*)ptr_aligned);
    }
#endif // DEBUG_TRACE_MEM
#endif // TRACE_EVENT_ENABLED

    if (err == 1)
    {
        printf ("Error 1: Bad %s:%s ofs=0x%X, be=0x%X, data=0x%X-%X!\n",
                name (), __FUNCTION__, (unsigned int) ofs, (unsigned int) be,
                (unsigned int) * ((uint32_t *) data + 0),
                (unsigned int) * ((uint32_t *) data + 1));
        //exit (1);
    }else if (err == 2){
        printf ("Error 2: Bad %s:%s ofs=0x%X, be=0x%X, data=0x%X-%X!\n",
                name (), __FUNCTION__, (unsigned int) ofs, (unsigned int) be,
                (unsigned int) * ((uint32_t *) data + 0),
                (unsigned int) * ((uint32_t *) data + 1));
    }
}

/**
 * @brief
 *
 * @param ofs
 * @param be
 * @param data
 * @param bErr
 */
void mem_device::read (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr)
{
    bErr = false;


    static bool onetime = true;
    if(onetime){
//        dump();
        onetime = false;
    }


    if (this->m_req.plen > 1)
    {
        uint32_t    be_off = 0;

        if(be == 0xF0)
            be_off = 4;

        *((uint32_t *) (data + be_off)) = ofs + be_off;

#ifdef TRACE_EVENT_ENABLED
        if(enable_trace){
            analyse_mem_shared(ofs + be_off, MEM_READ);
        }

        uintptr_t ptr = (uintptr_t)((uint8_t*)(mem+ofs));
        //    uintptr_t ptr_aligned = ptr &  ~0x3;

       if (m_req.hwe_cont_ref != 0 ){
#ifdef RABBITS_TRACE_EVENT_CACHE
           HWE_CACHE_mem_set_data((hwe_cont*)m_req.hwe_cont_ref,(uint8_t*)ptr);
#endif  
       }/*else if (enable_trace){
           // Create a fake request 
           hwe_cont *req = hwe_init(hwe_periph_port);
           HWE_MEM_mem_init(req, 0, HWE_MEM_LOAD, ofs, this->m_req.plen); 
           HWE_HEAD_set_date(req, 0, sc_time_stamp().value()); // begin date
           HWE_HEAD_set_date(req, 1, sc_time_stamp().value()); // end   data
           HWE_MEM32_set_data(req,(uint8_t*)ptr);
           HWE_MEM32_set_rcvack(req); // XXX: usefulless

           // Create a real ack!
           hwe_cont *ack = hwe_init(hwe_mem_port);
           HWE_MEM_ack_init(ack); 
           HWE_HEAD_set_child(req, ack);
           HWE_HEAD_set_ref(ack, 0, HWE_HEAD_this(req));
           HWE_MEM_ack_set_date(ack, sc_time_stamp().value());

           DCOMMIT(ack);
           hwe_commit(ack);
           DCOMMIT(req);
           hwe_commit(req);
#ifdef DEBUG_TRACE_MEM_READ
           printf("DEVICE READ\n");
#endif
       }*/
#ifdef DEBUG_TRACE_MEM_READ
       //  if ((ofs+(offset_dd*mod))%4){ 
       //      printf("@0x%08x => A@0x%08x = 0x%08x\n", (ofs+(offset_dd*mod))
       //                                             , (ofs+(offset_dd*mod))&(~0x3)
       //                                             , *(uint32_t*)ptr_aligned);
       //  } else {
       //        printf("@0x%08x = 0x%08x\n", (ofs+(offset_dd*mod))
       //                                   , *(uint32_t*)ptr_aligned);
       //  }
#endif // DEBUG_TRACE_MEM
#endif // TRACE_EVENT_ENABLED

       DPRINTF("read burst rsp: 0x%08x\n", * (uint32_t *) (data + be_off));

       wait (3 * this->m_req.plen, SC_NS);
    }
    else
    {
        int loffs = 0; /* data offset  */
        int lwid  = 0; /* access width */
        int err   = 0;

        if (ofs >= size || be == 0)
            err = 1;

        if (!err)
        {
            switch (be)
            {
            //byte access
            case 0x01: loffs = 0; lwid = 1; break;
            case 0x02: loffs = 1; lwid = 1; break;
            case 0x04: loffs = 2; lwid = 1; break;
            case 0x08: loffs = 3; lwid = 1; break;
            case 0x10: loffs = 4; lwid = 1; break;
            case 0x20: loffs = 5; lwid = 1; break;
            case 0x40: loffs = 6; lwid = 1; break;
            case 0x80: loffs = 7; lwid = 1; break;
            //word access
            case 0x03: loffs = 0; lwid = 2; break;
            case 0x0C: loffs = 1; lwid = 2; break;
            case 0x30: loffs = 2; lwid = 2; break;
            case 0xC0: loffs = 3; lwid = 2; break;
            //dword access
            case 0x0F: loffs = 0; lwid = 4; break;
            case 0xF0: loffs = 1; lwid = 4; break;
            default:
                err = 1;
            }

            if (!err)
                switch (lwid)
                {
                case 1:
                    *((uint8_t *)data + loffs) =
                        *((uint8_t *)(mem + ofs) + loffs);
                    break;
                case 2:
                    *((uint16_t *)data + loffs) =
                        *((uint16_t *)(mem + ofs) + loffs);
                    break;
                case 4:
                    *((uint32_t *)data + loffs) =
                        *((uint32_t *)(mem + ofs) + loffs);
                    break;
                default:
                    err = 1;
                }
        }

//        printf("read individual ofs: 0x%08x\t loffs: 0x%08x\n",ofs,loffs  );
       uintptr_t ptr = (uintptr_t)((uint8_t*)(mem+ofs));

       if (m_req.hwe_cont_ref != 0 ){
#ifdef RABBITS_TRACE_EVENT_CACHE
           HWE_CACHE_mem_set_data((hwe_cont*)m_req.hwe_cont_ref,(uint8_t*)ptr);
#endif  
       }

        if(err == 1)
        {
            printf("Bad %s:%s ofs=0x%X, be=0x%X, data=0x%X-%X!\n",
                    name(), __FUNCTION__, (unsigned int) ofs, (unsigned int) be,
                    *((uint32_t *)data + 0), *((uint32_t *)data + 1));
            bErr = true;
        }

        wait (3, SC_NS);
    }
}

/**
 * @brief
 *
 * @param ofs
 * @param be
 * @param data
 * @param bWrite
 */
void mem_device::rcv_rqst (uint32_t ofs, uint8_t be,
                           uint8_t *data, bool bWrite)
{
    bool bErr = false;

#ifdef TRACE_EVENT_ENABLED
    if (m_req.hwe_cont_ref != 0  && enable_trace == false){
        create_mem_init();
        create_mem_shared();
        enable_trace = true;
    }
#endif

    if (bWrite)
        this->write (ofs, be, data, bErr);
    else
        this->read (ofs, be, data, bErr);

#ifdef TRACE_EVENT_ENABLED
    this->tr_ack(data, ofs, be);
#endif
    send_rsp (bErr);
}

#ifdef TRACE_EVENT_ENABLED


/**
 *
 */
void mem_device::tr_ack (uint8_t *data, uint32_t ofs, uint8_t be)
{
    if (m_req.hwe_cont_ref != 0 ){        
        hwe_cont *ack = hwe_init(hwe_mem_port);
        hwe_cont *ref = (hwe_cont*)m_req.hwe_cont_ref;
        uint32_t addr = 0, width = 0;
        // Set Cache Acknowledgement
        // Memory Acknowledgement Event
        HWE_MEM_ack_init(ack);
#ifdef RABBITS_TRACE_EVENT_CACHE
        HWE_CACHE_mem_enddate(ref, sc_time_stamp().value());
        HWE_HEAD_set_ref(ack, 0, HWE_HEAD_this(ref));
#else
        hwe_cont* hwe_parent;
        hwe_parent = HWE_HEAD_get_parent(ref);
        HWE_CACHE_mem_enddate(hwe_parent, sc_time_stamp().value());
        HWE_HEAD_set_ref(ack, 0, HWE_HEAD_this(hwe_parent));
#endif  
        HWE_MEM_ack_set_date(ack, sc_time_stamp().value());
        // Access get from reference!!!
        if(!get_addr_width(ofs, be, &addr, &width)){
            cout << "Error ofs and be did not match" << endl; 
        }
        HWE_CACHE_ack_access(ack, ref->mem.body.mem32.access, addr, width);

        DCOMMIT(ack);
        hwe_commit(ack);
    }
}



/*================================================================================
 * MEMORY MODIFICATION FUNCTIONS 
 *================================================================================*/

/**
 * @brief 
 *
 * @param state
 * @param addr
 *
 * @return 
 */
uint8_t mem_device::mem_init(uint8_t state, uint32_t addr)
{
    static uint32_t sections = 0, section_size = 0, section_addr = 0 ;
    static uint8_t old_state = MEM_INIT_CONSTRUCT;
    char ptr[30];
    static FILE* fd = NULL; 

    switch (state){
    case(MEM_INIT_CONSTRUCT):
        fd = fopen("mem_init", "r+");
        if ( fd == NULL){
            cout << "The mem_init file does not exists. It must be first created by app_loader" << endl;
            exit(1);
        }
        fscanf(fd, "N_SECTIONS=%08x\n",&sections);
        // Alloc initial 
        state = MEM_INIT_SECTION;
        old_state = MEM_INIT_CONSTRUCT;
        break;
    case MEM_INIT_SECTION: // Initialize a new section
        sections++;
        section_size = 0x1F;
        section_addr = addr;
        state = MEM_INIT_APPEND;
        old_state = MEM_INIT_SECTION;

//        cout << "INIT_SECTION = " << sections << endl; 

        break;
    case MEM_INIT_APPEND: // Append the size of modified data
        section_size += 0x1F;
        old_state = MEM_INIT_APPEND;
        break;
    case MEM_INIT_CLOSE: // Store the modified data in mem_init file
        if(old_state == MEM_INIT_SECTION || old_state == MEM_INIT_APPEND){
            fseek(fd, 0, SEEK_SET);
            sprintf(ptr, "N_SECTIONS=%08x\n", sections);
            fwrite(ptr, strlen(ptr), 1, fd);

            fseek(fd,0, SEEK_END);
            sprintf(ptr, "SECTION=%08x\n", sections);
            fwrite(ptr, strlen(ptr), 1, fd);
            sprintf(ptr, "SIZE=%08x\n", section_size);
            fwrite(ptr, strlen(ptr), 1, fd);
            sprintf(ptr, "OFFSET=%08x\n", section_addr);
            fwrite(ptr, strlen(ptr), 1, fd);

            fwrite(mem + section_addr, section_size, 1, fd);

            state = MEM_INIT_SECTION;
            old_state = MEM_INIT_CLOSE;

//            cout << "CLOSE_SECTION = " << sections << endl;
//            cout << "SIZE          = " << section_size << endl;
//            cout << "OFFSET        = " << section_addr << endl;
        }else{
            state = MEM_INIT_SECTION;
            old_state = MEM_INIT_CLOSE;
        }

        break;
    case MEM_INIT_DESTROY:
        fclose(fd);
        state = MEM_INIT_DESTROY;
        old_state = MEM_INIT_DESTROY;
        break;
    default:
        break;
    }
    return state;
}

/**
 * @brief 
 */
void mem_device::create_mem_init(void)
{
    uint64_t cell = 0;
    uint32_t mod_mem_max = size >> 11, i,j; 
    uint32_t mem_init_state = mem_init(MEM_INIT_CONSTRUCT,0);

    for(i = 0; i < mod_mem_max; i++){ // Scan the chain of modifed addresses
        unordered_map<uint32_t,uint64_t>::const_iterator iter = mod_mem.find(i);
        if (iter != mod_mem.end()){ // An address in this cell was modified 
            cell = mod_mem[i];
            for(j = 0; j < 64; j++){
                if ( cell && (uint64_t)1 << j){ // This slot was modified
                    mem_init_state = mem_init(mem_init_state, i << 11 | j << 5);
                }else{
                    mem_init_state = mem_init(MEM_INIT_CLOSE, 0);
                }
            }
        }else{ // This cell is empty
            mem_init_state = mem_init(MEM_INIT_CLOSE, 0);
        }
    }
   mem_init(MEM_INIT_DESTROY, 0);
}

/**
 * @brief 
 *
 * @param addr
 */
void mem_device::analyse_mem_mod(uint32_t addr)
{
    uint64_t cell;
    uint32_t ofs_shift = addr >> 11;

    unordered_map<uint32_t,uint64_t>::const_iterator iter = mod_mem.find(ofs_shift);

    if (iter != mod_mem.end()){
        cell = mod_mem[ofs_shift];
    }else{
        cell = 0;
    }

    uint64_t ofs_mask_shift = (uint64_t)1 << (addr >> 5 & ((1 << 6) -1));
    cell = cell | ofs_mask_shift; // each element represents 32 address

    mod_mem[ofs_shift] = cell;
}

/**
 * @brief 
 *
 * @param ofs
 * @param be
 * @param addr
 * @param width
 *
 * @return 
 */
bool mem_device::get_addr_width(uint32_t ofs, uint8_t be, uint32_t* addr, uint32_t *width){
    
    uint8_t lwid, loffs;
    switch (be) {
    //byte access
    case 0x01: loffs = 0; lwid = 1; break;
    case 0x02: loffs = 1; lwid = 1; break;
    case 0x04: loffs = 2; lwid = 1; break;
    case 0x08: loffs = 3; lwid = 1; break;
    case 0x10: loffs = 4; lwid = 1; break;
    case 0x20: loffs = 5; lwid = 1; break;
    case 0x40: loffs = 6; lwid = 1; break;
    case 0x80: loffs = 7; lwid = 1; break;
    //word access
    case 0x03: loffs = 0; lwid = 2; break;
    case 0x0C: loffs = 1; lwid = 2; break;
    case 0x30: loffs = 2; lwid = 2; break;
    case 0xC0: loffs = 3; lwid = 2; break;
    //dword access
    case 0x0F: loffs = 0; lwid = 4; break;
    case 0xF0: loffs = 1; lwid = 4; break;
    default:
        return false;
    }

    *width = lwid;
    *addr = ofs + (loffs * lwid);

    return true;

}
/*================================================================================
 * MEMORY SHARED FUNCTIONS 
 *================================================================================*/

/**
 * @brief 
 */
void mem_device::create_mem_shared(void)
{
    char ptr[64];

    shared_fd = fopen("mem_shared", "r");

    if(shared_fd != NULL){
        fclose(shared_fd);
        printf("Mem shared problem: remove mem_shared file before start\n");
        exit(1);
    }else{
        shared_fd = fopen("mem_shared","w+");
    }

    sprintf(ptr, "ADDR\tWRITERS\tREADERS\n");
    fwrite(ptr, strlen(ptr), 1, shared_fd);
    cout << "mem_shared file created" << endl;

}

/**
 * @brief 
 *
 * @param addr
 * @param type
 */
void mem_device::analyse_mem_shared(uint32_t addr, bool type)
{

    shared_t comp;
    char ptr[64];
    addr &= CACHE_MASK;

    unordered_map<uint32_t,shared_t>::const_iterator iter = shared_mem.find(addr);

    switch (type){
    case MEM_READ:
        if (iter != shared_mem.end()){
            uint32_t reader;
            comp = shared_mem[addr];
            reader = m_req.hwe_cont_ref ?
                1 << (uint32_t)((hwe_cont*)m_req.hwe_cont_ref)->common.id.devid : // Processors
                1 << 31; // Peripheral

            if(comp.writers & ~reader){// Reader != Writer
                if( !((uint32_t)comp.readers & (uint32_t) reader)){                        
                    comp.readers |= reader;
                    //                        printf(" @ 0x%08x W 0x%08x R 0x%08x NEW READER",addr, comp.writers, comp.readers);
                    if(comp.fofs){
                        //                            printf(" OLD POSITION");
                        fseek(shared_fd, comp.fofs, SEEK_SET);
                    }else{
                        //                            printf(" NEW SHARED");
                        //                            printf(" END OF FILE");
                        fseek(shared_fd, 0, SEEK_END);
                        comp.fofs = ftell(shared_fd);
                    }
                    //                        printf("\n");

                    sprintf(ptr, "0x%08x 0x%08x 0x%08x\n",addr, comp.writers , comp.readers);
                    fwrite(ptr, strlen(ptr), 1, shared_fd);
                }
                shared_mem[addr] = comp;
            }
        }
    break;
    case MEM_WRITE:
        uint32_t writer;

        writer = m_req.hwe_cont_ref ?
            1 << (uint32_t)((hwe_cont*)m_req.hwe_cont_ref)->common.id.devid :
            1 << 31;

        if (iter != shared_mem.end()){
            comp = shared_mem[addr];
        }else{
            //            memset(&comp, 0, sizeof(shared_t));
            comp.writers = writer;
            comp.readers = 0;
            comp.fofs    = 0;
        }
        if(comp.fofs && !(comp.writers & writer )){
            //            printf(" @ 0x%08x W 0x%08x R 0x%08x NEW WRITER\n", addr, comp.writers, comp.readers);
            comp.writers |= writer;
            fseek(shared_fd, comp.fofs, SEEK_SET);
            sprintf(ptr, "0x%08x 0x%08x 0x%08x\n", addr, comp.writers , comp.readers);
            fwrite(ptr, strlen(ptr), 1, shared_fd);
        }
        shared_mem[addr] = comp;
    break;
    }
}

#endif

/*
 * Vim standard variables
 * vim:set ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:


*/
