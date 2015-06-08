/*
 *  Copyright (c) 2013 TIMA Laboratory
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

#ifdef TRACE_EVENT_ENABLED

#include <cfg.h>
#include <iostream>
#include <iomanip>
#include <qemu_imported.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <hwetrace.h>
#include <hwetrace_api.h>
#include <hwetrace_common.h>
#include <hwetrace_cache.h>
#include <hwetrace_processor.h>
#include <events/hwe_device.h>
#include <hwe_handle_def.h>

#include <../../qemu/sc_qemu/rabbits/systemc_imports.h>

#define SC_TR_DEBUG

#include "qemu_trace_port.h"
#include "../trace_port/trace_port.h"

using namespace std;

hwe_cont* hwe_par;

//#define COM_DEBUG
#ifdef COM_DEBUG
bool flag = false;
#define INDEX 13000000
#define DEV  2

#define DDEBUG(msg,hwe) if((((uint32_t)hwe->common.id.index > INDEX) && ((uint32_t)hwe->common.id.devid == DEV)) || flag){  \
   flag = true; \
   hwe_par = HWE_HEAD_get_parent(hwe); \
cout << dec <<left << setw(10) << msg << "[" << (uint32_t)hwe->common.id.devid << "." << left << setw(15) << (uint32_t)hwe->common.id.index << "]" ;\
if( hwe_par != 0)\
cout << "<-["<<(uint32_t)hwe_par->common.id.devid << "." << left << setw(15) <<(uint32_t)hwe_par->common.id.index << "] ";\
else cout << " 0x" <<setw(19) <<  hex <<hwe->inst.body.instr ;\
cout << __func__ << ":" << __LINE__ << endl;}

#else
#define DDEBUG(msg,hwe)
#endif

#define EDEBUG(msg,hwe) cout << dec << setw(10) << msg << "[" <<(uint32_t)hwe->common.id.devid << "." << (uint32_t)hwe->common.id.index << "]\t";\
hwe_par = HWE_HEAD_get_parent(hwe);\
if( hwe_par != 0)\
cout << "<-["<<(uint32_t)hwe_par->common.id.devid << "." << left << setw(15) <<(uint32_t)hwe_par->common.id.index << "] ";\
else cout << " 0x" <<setw(19) <<  hex <<hwe->inst.body.instr;\
cout << __func__ << ":" << dec << __LINE__ << endl;


#define SHOW_EVENT(hwe) { int i, nchild, com_child; \
    nchild = HWE_HEAD_get_nchild(hwe);              \
    com_child = HWE_HEAD_get_com_child(hwe);        \
    EDEBUG("\tINSTR",hwe);                          \
    printf("\t\tcom_child: %d\n\tnchild: %d\n",com_child,nchild);\
    for(i = com_child; i < nchild; i++){            \
        hwe_cpu_req = HWE_HEAD_get_child(hwe, i);   \
        if(hwe_cpu_req != NULL) {                   \
            EDEBUG("\tCPU_REQ",hwe_cpu_req);        \
            if(hwe_cpu_req != NULL){                    \
                if(HWE_HEAD_get_nchild(hwe_cpu_req)){   \
                    hwe_cache_req = HWE_HEAD_get_child(hwe_cpu_req,0); \
                    if(hwe_cache_req == NULL){          \
                        printf("CACHE_REQ error\n");        \
                    }else{                              \
                        EDEBUG("\tCACHE_REQ",hwe_cache_req);\
                    }                                       \
                }                                       \
            }                                           \
        }else{                                          \
           printf("\tNULL\n");                              \
        }                                               \
    }                                                   \
}

#define EFIFO_FULL(fifo) {hwe_cont* hwe = fifo->get(); cout << "ERROR: FIFO FULL" << endl; SHOW_EVENT(hwe)  }


#define DCOMMIT(hwe)   DDEBUG("COMMIT  ",hwe)
#define DHOLD(hwe)     DDEBUG("HOLD    ",hwe) 
#define DTRY_COM(hwe)  DDEBUG("TRY_COM ",hwe)
#define DSUCC_COM(hwe) DDEBUG("SUCC_COM",hwe)
#define DFAIL_MEM(hwe) DDEBUG("FAIL_MEM",hwe)
#define DFAIL_COM(hwe) DDEBUG("FAIL_COM",hwe)

#define COM_SUCC 0
#define COM_FAIL_WAIT_MEM_ACK 1
#define COM_FAIL_WAIT_COM_ACK 2

// TODO: Encapsulate the functions that manage the queue

/**
 * @brief
 *
 * @param qemu_inst
 * @param qi
 * @param cpu_index
 * @param cache
 */
qemu_trace_port::qemu_trace_port(qemu_instance *qemu_inst, qemu_import_t *qi,
                                 uint32_t cpu_index, cache_model_t *cache)
{
    cpuid            = cpu_index;
    qemu_import      = qi;
    buf              = qemu_import->qemu_get_tr_buf(qemu_inst, cpu_index);
    data             = (hwe_cont**)buf->data;
    held_events      = new fifo;
}

qemu_trace_port::~qemu_trace_port(void)
{
    delete held_events;
}

/**
 *
 */
bool qemu_trace_port::commit_held_events(void)
{
    uint8_t commited = false;
    do{
        hwe_inst = held_events->get();
        if(hwe_inst != NULL){
           DTRY_COM(hwe_inst);
           commited = commit_event();
           switch (commited){
           case COM_SUCC:
               DSUCC_COM(hwe_inst);
               held_events->update_tail();
               break;
           case COM_FAIL_WAIT_MEM_ACK:
               DFAIL_MEM(hwe_inst);
               hwe_inst = NULL;
               break;
           case COM_FAIL_WAIT_COM_ACK:
               DFAIL_COM(hwe_inst);
               hwe_inst = NULL;
               break;
           }
        } else {
            commited = true;
        }
    }while(hwe_inst != NULL );

    return commited;
}


/**
 * @brief 
 *
 * @param hwe
 *
 * @return 
 */
bool qemu_trace_port::commit_req_event(hwe_cont* hwe)
{
//    if (HWE_CACHE_mem_get_ack(hwe_cache_req)){   // CACHE - REQUEST
/*  int n_inval;
    n_inval = HWE_HEAD_get_nchild(hwe);
    
    if(n_inval){ // CACHE - INVALIDATIONS
        int j;
        for (j = 0; j < n_inval; j++){
            hwe_inval = HWE_HEAD_get_child(hwe,j);
            hwe_commit(hwe_inval);
        }
    }*/
    hwe_cont* hwe_held;
    hwe_cont* hwe_parent;

    HWE_CACHE_mem_set_ack(hwe);
    hwe_parent = HWE_HEAD_get_parent(hwe);

    if(hwe_parent == NULL){
        SHOW_EVENT(hwe);
        printf("No parent found!\n");
        exit(1);
    }

    HWE_CPU_dmem_enddate(hwe_parent, HWE_CACHE_mem_get_enddate(hwe));
    hwe_held = held_events->get();
    if(hwe_held == NULL /*|| hwe_inst == hwe_held*/) { // There is something wainting for an ack
        // Verify if it has a child
         
        if( HWE_HEAD_get_nchild(hwe_parent) < 1) {
            cout << HWE_HEAD_get_nchild(hwe_parent) << endl;
            EDEBUG("Req does not have childs. ",hwe_parent);
            EDEBUG("Request. ",hwe);
            SHOW_EVENT(hwe_parent);
            exit(1);
        }
/*        if( HWE_HEAD_get_nchild(hwe_cpu_req) != 1) {
            cout << HWE_HEAD_get_nchild(hwe_cpu_req) << endl;
            EDEBUG("Req has more childs than this one",hwe_cpu_req);
            exit(1);
        }else if(HWE_HEAD_get_nchild(hwe) != 0){
            cout << HWE_HEAD_get_nchild(hwe) << endl;
            EDEBUG("More childs than the memory ack",hwe);
            exit(1);
        }*/
//        HWE_HEAD_clean_child(hwe_cpu_req);
        HWE_HEAD_del_child(hwe_parent, hwe->common.child_slot);
#ifdef RABBITS_TRACE_EVENT_CACHE
        DCOMMIT(hwe);
        hwe_commit(hwe);
#else // !RABBITS_TRACE_EVENT_CACHE
        free(hwe);
#endif
    }else{
       commit_held_events();
    }
    return true;
}

/**
 * @brief
 *
 * @ie hwe_inst must be filled with a valid containner before call this function
 *
 * @return 
 */
uint32_t qemu_trace_port::commit_event(void)
{
    int i, nchild, com_child;
    nchild = HWE_HEAD_get_nchild(hwe_inst);
    com_child = HWE_HEAD_get_com_child(hwe_inst);

    for(i = com_child; i < nchild; i++){
#ifdef RABBITS_TRACE_EVENT_CPU_REQ
        hwe_cpu_req = HWE_HEAD_get_child(hwe_inst, i);
        if(hwe_cpu_req != NULL){
            if(HWE_HEAD_get_nchild(hwe_cpu_req)){ // IO Access doesnt have ACK
                if(HWE_HEAD_get_nchild(hwe_cpu_req) > 1){
                    EDEBUG("More children hidden",hwe_cpu_req);
                    exit(1);
                }
                hwe_cache_req = HWE_HEAD_get_child(hwe_cpu_req,0);
#else // !RABBITS_TRACE_EVENT_CPU_REQ
        hwe_cache_req = HWE_HEAD_get_child(hwe_inst,i);
#endif // RABBITS_TRACE_EVENT_CPU_REQ
        if(hwe_cache_req != NULL){
            if(HWE_MEMACK == HWE_HEAD_get_type(hwe_cache_req)){ // CACHE - MEMACK
                DCOMMIT(hwe_cache_req);
                hwe_commit(hwe_cache_req);
            }else if (HWE_CACHE_mem_get_ack(hwe_cache_req)){
#ifdef RABBITS_TRACE_EVENT_CACHE
                DCOMMIT(hwe_cache_req);
                hwe_commit(hwe_cache_req);
#else  // !RABBITS_TRACE_EVENT_CACHE
                free(hwe_cache_req);
#endif
            } else if(!HWE_CACHE_mem_get_ack(hwe_cache_req)){   // CACHE - REQUEST
                hwe_cont *held_event = held_events->get();
                if(held_event == NULL ||
                   held_event != hwe_inst){
                     DHOLD(hwe_inst);
                     if(!held_events->put(hwe_inst)){
                         EFIFO_FULL(held_events);
                         exit(1);
                     }
                }
                HWE_HEAD_set_com_child(hwe_inst,i);
                return COM_FAIL_WAIT_MEM_ACK; // STOP the commiting process
                // There is an event wainting for an ACK
             }else{
                EDEBUG("Previous problems",hwe_cache_req);
                SHOW_EVENT(hwe_cache_req);
                exit(1);
             }
        }
#ifdef RABBITS_TRACE_EVENT_CPU_REQ
            }
            DCOMMIT(hwe_cpu_req);
            hwe_commit(hwe_cpu_req);
        }
#endif // RABBITS_TRACE_EVENT_CPU_REQ
    }
    if(!hwe_inst->inst.ack_commit){ // STOP the commiting process
       hwe_inst->common.head.com_child = 0;
       hwe_inst->common.head.nchild = 0;
        return COM_FAIL_WAIT_COM_ACK;
    }
    DCOMMIT(hwe_inst);
    hwe_commit(hwe_inst);
    return COM_SUCC;
}

/**
 * @brief This function gets the number of cycles in one event and updates
 * the timestamp of such event. This function must be use in conjunction with
 * sync timing systemC
 * @return number of cycles
 */
uint16_t qemu_trace_port::get_event_cycles(uint64_t timestamp)
{
    uint16_t n_cycles = 0;
    hwe_cont* hwe_held;

    do {
        cont = get_event_queue();

        if (cont !=  NULL ){
            switch(HWE_HEAD_get_type(cont)){
            case HWE_INST32:
                HWE_CPU_inst_set_date(cont, timestamp);
                n_cycles = HWE_CPU_inst_get_cycles(cont);
                hwe_held = held_events->get();
                if(hwe_held != NULL) {
                    DHOLD(cont);
                    if(!held_events->put(cont)){
                        EFIFO_FULL(held_events);
                        exit(1);
                     }
                }
                break;
            case HWE_CPU_MEM:
            case HWE_MEM32:
            case HWE_CPU_IO:
                HWE_CACHE_mem_begdate(cont, timestamp);
                HWE_CACHE_mem_enddate(cont, timestamp);
                break;
            case HWE_MEMACK:
                hwe_cont* hwe_parent;
                   
                HWE_CACHE_ack_date(cont, timestamp);
                hwe_parent = HWE_HEAD_get_parent(cont); // hwe_cpu_req or hwe_inst
                HWE_CPU_dmem_enddate(hwe_parent, HWE_CACHE_mem_get_begdate(cont));
                // Try to commit event
                hwe_held = held_events->get();
                if(hwe_held == NULL) { // There is something wainting for an ack
                    HWE_HEAD_del_child(hwe_parent, cont->common.child_slot);
                    DCOMMIT(cont);
                    hwe_commit(cont);
                }
                break;
            case HWE_COMMIT:
                hwe_inst = (hwe_cont*)cont->common.parent;
                if (hwe_inst != NULL) {
                    hwe_inst->inst.ack_commit = 1;
                    hwe_held = held_events->get();
                    if(hwe_held == NULL){
                        commit_event();
                    }else/*if( hwe_inst == hwe_held )*/{
                        commit_held_events();
                    }
                }else {
                    EDEBUG("Commit_Event without a Inst_Event ", cont);
                    exit(1);
                }
                free(cont); // Just for COMMIT events
                break;
            default:
                EDEBUG("Event not recognized", cont);
                exit(1);
                break;
            }
        }
    } while((0 == n_cycles) && (NULL != cont));

    return n_cycles;
}

//******************************************************************************
//                        QUEUE MANAGEMENT
//******************************************************************************
/**
 * @brief
 *
 * @param[out] event Pointer to event
 *
 * @return The number of bytes received or -1 if an error occurs. If the buffer
 * is empty this function also returns -1.
 */
hwe_cont* qemu_trace_port::get_event_queue(void)
{
    uint32_t tail = 0; 
    if(!is_empty()){
        tail = buf->tail;
        update_tail();
        return data[tail];
    }
    return NULL;
}
/**
 * @brief Just verify if the buffer is empty
 * @return returns 1 if the buffer is empty and 0 otherwise
 */
bool qemu_trace_port::is_empty(void)
{
    return (buf->head == buf->tail) && (buf->last_op == TR_BUF_READ);
}

/**
 * @brief This function updates the tail variable of circular buffer
 */
void qemu_trace_port::update_tail(void)
{
    buf->tail++;
    buf->tail &= buf->size -1;
}

//******************************************************************************
//                        LIST MANAGEMENT
//******************************************************************************

//*****************
// FIFO
//*****************

/**
 *
 */
fifo::fifo(void)
{
    head  = 0;
    tail  = 0;
    size  = 0x1000;
    data  = (hwe_cont**) calloc(size, sizeof(hwe_cont*));
}

fifo::~fifo(void)
{
    free(data);
}

/**
 *
 * @param item pointer to copy the element
 * @return NULL if FIFO is empty, and item pointer if FIFO has item
 */
hwe_cont* fifo::get(void)
{
    if(head == tail && last_op == TR_BUF_READ){ // is empty
       return (hwe_cont*) NULL;
    }
    return data[tail];
}

void fifo::update_tail(void){
    data[tail] = 0;
    tail++;
    tail &= size -1;
    last_op = TR_BUF_READ;
}

/**
 * @param item pointer to be included in fifo
 * @return false if fifo is full or true if ok
 */
bool fifo::put(hwe_cont* item)
{
    if(head == tail && last_op == TR_BUF_WRITE){ // is full?
        return false;
    }
    data[head] = item;
    head++;
    head &= size - 1;
    last_op = TR_BUF_WRITE;
    return true;
}

#endif /* TRACE_EVENT_ENABLED */
