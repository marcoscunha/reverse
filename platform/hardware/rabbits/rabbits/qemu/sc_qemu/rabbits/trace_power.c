/**
 * @file trace_power.c
 *
 * @author marcos cunha marcos.cunha@imag.fr
 *
 * @version 1.0
 *
 * Copyright (c) 2013 TIMA Laboratory
 *
 * @section DESCRIPTION
 *
 * This file is part of Rabbits.
 *
 * @section LICENSE
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
 *
 *   ...
 *
 */

#include "cfg.h"

#ifdef RABBITS_TRACE_EVENT

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include "rabbits/qemu_systemc.h"
#include "fc_annotations.h"

#include <hwetrace.h>
#include <hwetrace_api.h>
#include <hwetrace_common.h>
#include <hwetrace_cache.h>
#include <hwetrace_processor.h>
#include <events/hwe_device.h>
#include "../../../components/trace_port/trace_port.h"

#include "trace_power.h"

#include "cpu.h"

#define DEBUG_EXEC_COMMIT

#ifdef DEBUG_EXEC_COMMIT
uint8_t dbg_commit[16] = {0,0,0,0,0,0,0,0,
                          0,0,0,0,0,0,0,0};
hwe_cont* dbg_hwe[16] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                         NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
#define DGB_SET_COMMIT(cpu,hwe) dbg_commit[cpu]++; \
                            dbg_hwe[cpu] = dbg_commit[cpu] > 1 ? dbg_hwe[cpu] : hwe;/*\
                          printf("[%d.%d] - SET EXEC COMMIT\n",(uint32_t)hwe->common.id.devid,\
                                                               (uint32_t)hwe->common.id.index);*/

#define DBG_CLR_COMMIT(cpu) dbg_commit[cpu]--
#define DBG_TEST_COMMIT(cpu)   if(dbg_commit[cpu]> 1){ \
                                printf("Commit Error !!!\n");\
                                printf("\t[%d.%d] 0x%08x @ 0x%08x\n", (uint32_t)dbg_hwe[cpu]->common.id.devid, \
                                                                      (uint32_t)dbg_hwe[cpu]->common.id.index, \
                                                                      dbg_hwe[cpu]->inst.body.instr, \
                                                                      dbg_hwe[cpu]->inst.body.pc);  \
                                exit(1);\
                            }/*else{\
                               printf("[%d.%d] - TEST EXEC COMMIT\n",(uint32_t)dbg_hwe[cpu]->common.id.devid,\
                                                                     (uint32_t)dbg_hwe[cpu]->common.id.index);\
                            }*/
#else
#define DGB_SET_COMMIT(cpu,hwe) 
#define DBG_CLR_COMMIT(cpu)
#define DBG_TEST_COMMIT(cpu)
#endif



//#define TR_DEBUG
extern bool enable_trace;

extern int          dcache_line_bytes;

static int arm_default_instr_cycle[] =
{
    /*ARM_NORMAL_INSTRUCTION*/               1,
    /*ARM_JUMP*/                             2,
    /*ARM_LOAD*/                             0,
    /*ARM_STORE*/                            0,
    /*ARM_SIGNED_MUL*/                       0,
    /*ARM_REGISTER_SHIFT*/                   0,
    /*ARM_MUL8*/                             0,
    /*ARM_MUL16*/                            0,
    /*ARM_MUL24*/                            0,
    /*ARM_MUL32*/                            0,
    /*ARM_MUL64*/                            0,
    /*ARM_MLS*/                              0,
    /*ARM_MLA*/                              0,
    /*ARM_MLAA*/                             0,
    /*ARM_SWP*/                              0,
    /*ARM_MULTI_TRANSFER_PER_REGISTER*/      1,
    /*ARM_MULTI_TRANSFER_LOAD_OP*/           1,
    /*ARM_MULTI_TRANSFER_STORE_OP*/          1,
    /*ARM_COCPU*/                            0,
    /*ARM_COCPU_MRC*/                        0,
};

#define USER_REGS 0b00010000
#define FIQ_MODE  0b00100000

#define CSPR_REG 0xFF
#define SPSR_REG 0xFE

#ifdef RABBITS_PERF
extern clock_t cost_execution;
#endif

// Global variable to avoid reallocation during runtime
trace_port_t** port;
//hwe_cont*     hwe_inst;
hwe_cont*     hwe_req;
hwe_cont*     hwe_mem;
hwe_cont*     hwe_ack;
hwe_head_cont*     hwe_com;
int icache_line_bytes;

uint32_t cpsr[16]  = {0,0,0,0,0,0,0,0,
                      0,0,0,0,0,0,0,0};

FILE* fdump;

/**
 * @brief Trace port initialization
 *
 * The infrastructure for trace port is created by this function, the queue
 * initialization for trace events
 *
 * @return 0 if initialization was success and -1 if an error occurred
 */
uint32_t tr_init_trace(trace_port_t** trace_port)
{
    fdump = fopen("dump", "w+");

    port = trace_port;
    icache_line_bytes = dcache_line_bytes;
    
	if( -1 == tr_init_queue()){
		return -1;
	}
	return 0;
}
void dump_regs(uint32_t pc);


void dump_regs(uint32_t pc)
{
    int i;
    static int dump = 0;
    dump++;
    if( dump == 1){
        dump = 0;
        fprintf(fdump, "pc = 0x%08x\n", pc);

        fprintf(fdump, "cspr = 0x%08x\n", cpsr_read(cpu_single_env));
        fprintf(fdump, "spsr = 0x%08x\n", cpu_single_env->spsr);

        for(i=0;i<15;i++){  // PC Excluded
            fprintf(fdump, "reg[%d] = 0x%08x\n", i, cpu_single_env->regs[i]);
        }        
        for(i=0;i<6;i++){
            fprintf(fdump, "banked_r13[%d]  = 0x%08x\n", i, cpu_single_env->banked_r13[i]);
            fprintf(fdump, "banked_r14[%d]  = 0x%08x\n", i, cpu_single_env->banked_r14[i]);
            fprintf(fdump, "banked_spsr[%d] = 0x%08x\n", i, cpu_single_env->banked_spsr[i]);
        }
        for(i=0;i<5;i++){
            fprintf(fdump, "fiq_regs[%d] = 0x%08x\n", i, cpu_single_env->fiq_regs[i]);
        }
        for(i=0;i<5;i++){
            fprintf(fdump, "usr_regs[%d] = 0x%08x\n", i, cpu_single_env->usr_regs[i]);
        }
        fprintf(fdump, "\n");
    }
}


/**
 * @brief Write Instruction Event
 *
 * It init a instruction event, it writes basic information on init event such
 * as identification, type of operation, and so on,  but some information are
 * filled just in TLM level, for instance,  timestamp.
 *
 * @param  cpu CPU which generates the event
 * @param  pc Program Counter Instruction
 * @param  insn Intruction executed
 * @param  inst_grp Group Instruction ( JUMP, NORMAL_OPERATION, ...)
 *
 * @return -1 if a error occurred writing the fifo or event ID if its was
 * created successfully
 */
__inline__ hwe_cont* tr_wr_inst_event(uint32_t cpu, target_ulong pc,
                                     target_ulong insn,
                                     tr_inst_t inst_grp,
                                     target_ulong dest)
{
    hwe_cont* hwe_inst;
    hwe_inst = hwe_init(port[cpu]->cpu);
    HWE_CPU_inst_init(hwe_inst, pc, 4);
    HWE_CPU_inst_set_instr(hwe_inst, insn);

    switch(inst_grp){
    case TR_ARM_NORMAL_INSTRUCTION:
       tr_exec_analysis(hwe_inst); // The hwe_inst must be set before
       break;
    case TR_ARM_JUMP:
//       tr_jump_analysis(insn);
/*        if(cond != 0xe ){
            event->cond = 1;
            if(!tr_test_cc(cond)){
                event->exec = 0;
                return 0;
            }
        }
        event->cycle += arm_default_instr_cycle[TR_ARM_JUMP];
        event->imem_access += 2;*/
        break;
    case TR_ARM_COCPU_MRC:
//       tr_arm_cocpu_analysis(insn);
/*        // TODO: CP15 processor instructions
        //cycles += arm_default_instr_cycle[TR_ARM_JUMP];
        event->imem_access = 1;*/
        break;
    default:
        return NULL;
        break;
    }
    DGB_SET_COMMIT(cpu_single_env->cpu_index,hwe_inst);
    return tr_put_event_queue (cpu, hwe_inst);
}

/**
 * @brief Write Request Event
 *
 * This function fills the request event and write it on queue for
 * TLM consumption
 *
 * @param cpu CPU that generates the event
 * @param src_id Source Event identification
 * @param addr Access Address
 * @param type Type of Event (Request from CACHE, from CPU, ... )
 * @param access Write or Read access
 * @return-1 if a error occurred writing the FIFO or event ID if its was
 * created successfully
 */
__inline__ hwe_cont* tr_wr_req_event(uint32_t cpu, hwe_cont* hwe_src, uint32_t addr,
                        tr_event_grp_t type, tr_mem_t access)
{
    hwe_port_t* req_port;

    if(hwe_src == NULL) return NULL;

    switch(type){
    case TR_EVNT_PROC_ICACHE_REQ:
        HWE_HEAD_add_expected(hwe_src, 1);
        req_port = port[cpu]->cpu;
        hwe_req = hwe_init(req_port);
        HWE_CPU_imem_init(hwe_req, 1, addr, 4);
        break;
    case TR_EVNT_PROC_DCACHE_REQ:
        HWE_HEAD_add_expected(hwe_src, 1);
        req_port = port[cpu]->cpu;
        hwe_req = hwe_init(req_port);
        HWE_CPU_dmem_init(hwe_req,1,access,addr,4); 
       break;
    case TR_EVNT_ICACHE_REQ:
#ifndef RABBITS_TRACE_EVENT_CPU_REQ
        HWE_HEAD_add_expected(hwe_src, 1);
#endif
#ifdef RABBITS_TRACE_EVENT_CACHE
        req_port = port[cpu]->icache;
        hwe_req = hwe_init(req_port);
        HWE_CACHE_mem_init(hwe_req, 1, access, addr, icache_line_bytes); // Expected is putted here
#else
      hwe_req = (hwe_cont*)calloc(1,sizeof(hwe_cont));
//        hwe_req = (hwe_cont*)malloc(sizeof(hwe_cont));
        HWE_SPARE_init(hwe_req);
#endif
        break;
    case TR_EVNT_DCACHE_REQ:
#ifndef RABBITS_TRACE_EVENT_CPU_REQ
        HWE_HEAD_add_expected(hwe_src, 1);
#endif        
#ifdef RABBITS_TRACE_EVENT_CACHE
       req_port = port[cpu]->dcache;
       hwe_req = hwe_init(req_port);
       HWE_CACHE_mem_init(hwe_req, 1, access, addr, dcache_line_bytes); // Expected is putted here
#else
       hwe_req = (hwe_cont*)calloc(1,sizeof(hwe_cont));
//       hwe_req = (hwe_cont*)malloc(sizeof(hwe_cont));
       HWE_SPARE_init(hwe_req);
#endif // RABBITS_TRACE_EVENT_CACHE

       break;
    case TR_EVNT_PROC_IO_REQ:
       req_port = port[cpu]->cpu;
       hwe_req = hwe_init(req_port);
       HWE_CPU_io_init(hwe_req, 0, addr, 1);
       break;
    default:
       printf("%s: Event %i was not recognized event\n", __FUNCTION__, type);
       return NULL;
       break;
    }

    HWE_HEAD_set_child(hwe_src, hwe_req);
#ifdef RABBITS_TRACE_EVENT_CACHE 
    HWE_HEAD_set_ref(hwe_req, 0, HWE_HEAD_this(hwe_src));

    return tr_put_event_queue (cpu, hwe_req);
#else
    return hwe_req;
#endif
}
/**
 *
 */
__inline__ hwe_cont* tr_wr_str_event(uint32_t cpu, hwe_cont* hwe_src, uint32_t addr,
                        tr_event_grp_t type, unsigned int width)
{

#ifdef RABBITS_TRACE_EVENT_CACHE
    hwe_port_t* req_port;
#endif

    if(hwe_src == NULL) return NULL;

    switch(type){
    // Include here the requests 
    case TR_EVNT_DCACHE_WRITEBACK:
//        printf("STR addr 0x%08x width %d\n",addr,width);
    case TR_EVNT_DCACHE_WRITE_HIT:
    case TR_EVNT_DCACHE_WRITE_MISS:

#ifndef RABBITS_TRACE_EVENT_CPU_REQ
       HWE_HEAD_add_expected(hwe_src, 1);
#endif
#ifdef RABBITS_TRACE_EVENT_CACHE
       req_port = port[cpu]->dcache;
       hwe_req = hwe_init(req_port);
       HWE_CACHE_mem_init(hwe_req, 1, TR_MEM_STORE, addr, width); // Expected is putted here
#else // !RABBITS_TRACE_EVENT_CACHE
   hwe_req = (hwe_cont*)calloc(1,sizeof(hwe_cont));
//       hwe_req = (hwe_cont*)malloc(sizeof(hwe_cont));
       HWE_SPARE_init(hwe_req);
#endif 
       break;
    default:
       printf("%s:%s: Not supported event\n",__FILE__,__FUNCTION__);
       exit(1);
    break;
    }

    // Including the references in ref
    HWE_HEAD_set_child(hwe_src, hwe_req);

#ifdef RABBITS_TRACE_EVENT_CACHE 
    HWE_HEAD_set_ref(hwe_req, 0, HWE_HEAD_this(hwe_src));

	return tr_put_event_queue (cpu, hwe_req);
#else
    return hwe_req;
#endif

}

/**
 * @brief Write a Acknowledge Event
 *
 * This function fills the acknowledge event and write on queue to TLM
 * consumption
 *
 * @param cpu CPU that generates the event
 * @param src_id Source Event identification that made the request
 * @param addr Access Address
 * @param type Type of acknowledge (from CACHE, MEMORY or PERIPHERALS)
 * @return
 */
__inline__ hwe_cont* tr_wr_ack_event(uint32_t cpu, hwe_cont* hwe_src, uint32_t addr, uint32_t width,
                                     tr_inst_t type, hwe_mem_t access)
{
    hwe_port_t* ack_port;

    if(hwe_src == NULL ) return NULL;

    switch(type){
    case TR_EVNT_DCACHE_MODIFY:
//        printf("MOD addr 0x%08x width %d\n",addr,width);
    case TR_EVNT_DCACHE_ACK:
        ack_port = port[cpu]->dcache;
#ifndef RABBITS_TRACE_EVENT_CPU_REQ
        HWE_HEAD_add_expected(hwe_src, 1);
#endif        
        break;
    case TR_EVNT_ICACHE_ACK:
        ack_port = port[cpu]->icache;
#ifndef RABBITS_TRACE_EVENT_CPU_REQ
        HWE_HEAD_add_expected(hwe_src, 1);
#endif
        break;
    default:
        printf("%s: Event %i was not recognized event\n", __FUNCTION__, type);
        return NULL;
    }

    hwe_ack = hwe_init(ack_port);

    HWE_HEAD_set_child(hwe_src, hwe_ack);
    HWE_CACHE_ack_init(hwe_ack, HWE_HEAD_this(hwe_src));
    HWE_CACHE_ack_access(hwe_ack, access , addr, width);

    return tr_put_event_queue (cpu, hwe_ack);
}

/**
 *
 * @param cpu CPU that generates the event
 * @param src_id Source Event identification that made the request
 * @param addr Access Address
 *
 */
__inline__ hwe_cont* tr_wr_invalidate_event(uint32_t cpu, uint32_t addr, tr_event_grp_t type, hwe_cont* hwe_src, hwe_date_t timestamp)
{
    if(hwe_src == NULL) return NULL;
/*    switch(type){
        case TR_EVNT_DCACHE_INV:
            hwe_mem  = hwe_init(port[cpu]->dcache);
            HWE_CACHE_mem_init (hwe_mem, 0, HWE_MEM_INVAL, addr, dcache_line_bytes);
        break;
        case TR_EVNT_ICACHE_INV:
            hwe_mem  = hwe_init(port[cpu]->icache);
            HWE_CACHE_mem_init (hwe_mem, 0, HWE_MEM_INVAL, addr, icache_line_bytes);
        break;
        case TR_EVNT_DCACHE_REPL:
            hwe_mem  = hwe_init(port[cpu]->dcache);
            HWE_CACHE_mem_init (hwe_mem, 0, HWE_MEM_REPL, addr, dcache_line_bytes);
            break;
        case TR_EVNT_ICACHE_REPL:
            hwe_mem  = hwe_init(port[cpu]->icache);
            HWE_CACHE_mem_init (hwe_mem, 0, HWE_MEM_REPL, addr, icache_line_bytes);
        break;
        case TR_EVNT_DCACHE_SW_INV:
            hwe_mem  = hwe_init(port[cpu]->dcache);
            HWE_CACHE_mem_init (hwe_mem, 0, HWE_MEM_SW_INVAL, addr, dcache_line_bytes);
            break;
        case TR_EVNT_ICACHE_SW_INV:
            hwe_mem  = hwe_init(port[cpu]->icache);
            HWE_CACHE_mem_init (hwe_mem, 0, HWE_MEM_SW_INVAL, addr, icache_line_bytes);
        break;
        case TR_EVNT_DCACHE_SW_FLUSH:
            hwe_mem  = hwe_init(port[cpu]->dcache);
            HWE_CACHE_mem_init(hwe_mem, 0, HWE_MEM_SW_FLUSH, addr, dcache_line_bytes);
        break;

        default:
            printf("%s: Event %i was not recognized event\n", __FUNCTION__, type);
            return;
        break;
    }

    HWE_HEAD_set_expected(hwe_mem, 0);//self ack

    if (timestamp){
       HWE_CPU_dmem_begdate(hwe_mem, timestamp);
       HWE_CPU_dmem_enddate(hwe_mem, timestamp);
       if(hwe_src == NULL){
          hwe_commit(hwe_mem);
       }
    } else{
       tr_put_event_queue(cpu, (hwe_cont*)hwe_mem);
    }

    if (hwe_src){
        // Link with ACK just to be commited
        HWE_HEAD_set_child(hwe_src, hwe_mem);
    }*/
    return NULL;
}

/**
 *
 * @param cpu
 * @param insn
 */
__inline__ hwe_cont* tr_wr_commit_event(uint32_t cpu, target_ulong pc_after)
{
    int i = 0;

    hwe_cont *hwe_tr_inst = cpu_single_env->rabbits.tr_id;

    hwe_com = (hwe_head_cont*)calloc(1,sizeof(hwe_head_cont));

    HWE_COMMIT_init((hwe_cont*)hwe_com);
    hwe_com->parent = (hwe_head_cont*)hwe_tr_inst;

    // Verify if it is a jumper
    if(HWE_CPU_inst_is_jump(hwe_tr_inst)){
        HWE_CPU_inst_set_jump(hwe_tr_inst, pc_after);
    }
    // Verify if this instruction modify a register value!!
    if(HWE_CPU_inst_has_reg(hwe_tr_inst)){
        int nreg, reg;
        nreg = HWE_CPU_inst_get_nreg(hwe_tr_inst);
        for(i = 0; i < nreg; i++){

            reg =  HWE_CPU_inst_get_regid(hwe_tr_inst, i);
            if(reg < 16){ // CURRENT REGISTERS
                HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->regs[reg]);
            }else if(reg == CSPR_REG){
                HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpsr_read(cpu_single_env));
                cpsr[cpu] = cpsr_read(cpu_single_env); // To avoid overwrite with below test(*) 
            }else if(reg == SPSR_REG){
                HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->spsr);
            } else if(reg & USER_REGS){ // USER REGS
                reg &= ~USER_REGS;
                if( reg < 8) {
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->regs[reg]);
                } else if (reg < 13 ){
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->usr_regs[reg-8]);
                } else if (reg == 13 ){
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->banked_r13[0]);
                } else if (reg == 14 ){
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->banked_r14[0]);
                } else{ goto reg_error; }

            } else if(reg & FIQ_MODE){
                reg &= ~FIQ_MODE;
                if( reg < 8) {
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->regs[reg]);
                } else if (reg < 13) {
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->fiq_regs[reg-8]);
                } else if (reg == 13 ){
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->banked_r13[5]);
                } else if (reg == 14 ){
                    HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->banked_r14[5]);
                } else{ goto reg_error; }
            }else {
                reg_error:
//                TRACE_PRINT(hwe_tr_inst);
                printf("event [%d.%d] \n", (uint32_t)hwe_tr_inst->common.id.devid, (uint32_t)hwe_tr_inst->common.id.index);
                printf("insn 0x%08x @0x%08x\n", hwe_tr_inst->inst.body.instr,  hwe_tr_inst->inst.body.pc);
                printf("register reg[%d] = %x did not recognize from cpu [%d]\n", i, reg, cpu);
                exit(1);
            }          
        }
    }
        
    if (cpsr[cpu] != cpsr_read(cpu_single_env)){ // Test (*)
        uint32_t mode, mode_old;
        mode = cpsr_read(cpu_single_env) & CPSR_M;
        mode_old = cpsr[cpu] & CPSR_M;

        cpsr[cpu] = cpsr_read(cpu_single_env);
        HWE_CPU_inst_add_reg(hwe_tr_inst);
        HWE_CPU_inst_set_regid(hwe_tr_inst, i, CSPR_REG);
        HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpsr_read(cpu_single_env));
        i++;

        // Verify if the mode was changed! // Thinking about to put this at the beggining 
        // of instruction execution
        if(mode != mode_old){
            uint32_t j;
            if(mode_old == ARM_CPU_MODE_FIQ || mode == ARM_CPU_MODE_FIQ){
                for(j=8;j<13;j++,i++){
                    HWE_CPU_inst_add_reg(hwe_tr_inst);
                    HWE_CPU_inst_set_regid(hwe_tr_inst, i, j);
                       HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->regs[j]);
                }
            }   
            HWE_CPU_inst_add_reg(hwe_tr_inst);
            HWE_CPU_inst_set_regid(hwe_tr_inst, i, 13);
            HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->regs[13]);
            i++;
            HWE_CPU_inst_add_reg(hwe_tr_inst);
            HWE_CPU_inst_set_regid(hwe_tr_inst, i, 14);
            HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->regs[14]);
            i++;
            HWE_CPU_inst_add_reg(hwe_tr_inst);
            HWE_CPU_inst_set_regid(hwe_tr_inst, i, SPSR_REG);
            HWE_CPU_inst_set_regdata(hwe_tr_inst, i, cpu_single_env->spsr);
        }
    }
    return tr_put_event_queue(cpu, (hwe_cont*)hwe_com);
}

/**
 * @brief commit
 * Classify where are comming the commit from a gen_jmp or a normal jumper!
 * @param pc_after address of PC after instruction execution
 * @param is_jmp type of jumper
 */
void tr_commit(target_ulong pc_after, target_ulong is_jmp)
{
    // Generate a new event in the end o events execution !!!
    int cpu = cpu_single_env->cpu_index;
    int32_t pc = 0x00;

    DBG_TEST_COMMIT(cpu_single_env->cpu_index);
    DBG_CLR_COMMIT(cpu_single_env->cpu_index);

    switch(is_jmp){
    case DISAS_NEXT:
        pc = pc_after;
    break;
    case DISAS_JUMP:
        pc = cpu_single_env->regs[15];
    break;
    case DISAS_UPDATE:
        pc = cpu_single_env->regs[15];
    break;
    case DISAS_TB_JUMP:
        pc = pc_after;
    break;
    case 4: //DISAS_SWI:
        pc = cpu_single_env->regs[15]; // To be confirmed
    break;
    case 5: //DISAS_WFI:
        pc = cpu_single_env->regs[15]; // To be confirmed
    break;
    default:
    break;
    }

    if(!tr_wr_commit_event(cpu, pc)){
        printf("%s:%d - Buffer is full\n", __FUNCTION__, __LINE__);
        exit(1);
    }
}

//******************************************************************************
//                             QUEUE MANAGEMENT
//******************************************************************************

/**
 * @brief Function to create a shared event queue
 * This function created some ring buffer queues to store the events based on
 * number of processors created by the QEMU.
 * @return 0 if success or -1 if a memory allocation error occurs.
 */
uint32_t tr_init_queue (void)
{
    int cpu;
    uint32_t n_cpu = crt_qemu_instance->m_NOCPUs;
    qemu_tr_buf_t* tr_buf;

    tr_buf = malloc(n_cpu * sizeof(qemu_tr_buf_t));
    if( NULL == tr_buf ){
        return -1;
    }

    crt_qemu_instance->m_tr_buf = tr_buf;

    for(cpu = 0; cpu < n_cpu; cpu++){
        tr_buf->size    = TR_EVNT_BUF_SIZE;
        tr_buf->head    = 0;
        tr_buf->tail    = 0;
        tr_buf->data    = malloc(sizeof(tr_event_t*) * tr_buf->size); /** TODO: Test Malloc */
        tr_buf->last_op = 0; //
        tr_buf->timestamp_updated = 0;
        tr_buf++;
    }
    return 0;
}

/**
 * @brief Write queue function
 * This function put the event on adequated queue based on cpu number
 * @param[in] tr_event event to be stored
 * @return Pointer to event
 */
__inline__ hwe_cont* tr_put_event_queue (uint32_t cpu, hwe_cont* cont)
{
    qemu_tr_buf_t* tr_buf = &crt_qemu_instance->m_tr_buf[cpu];

    if(!tr_is_full_queue(tr_buf)){
         tr_buf->data[tr_buf->head] = cont;
         tr_update_head_queue(tr_buf);
         return cont;
    }
    return NULL;
}

/**
 * @brief Verify if the queue is full
 * Test if tail is equal to head and the last operation was a write to
 * determine if the buffer is full.
 * @param tr_buf Queue to be verified
 * @return 1 if the queue is full or 0 if there is/are empty slots.
 */
__inline__ uint8_t tr_is_full_queue(qemu_tr_buf_t* tr_buf)
{
    return (tr_buf->head == tr_buf->tail) && (tr_buf->last_op == TR_BUF_WRITE);
}

/**
 * @brief The function increments the buffer's head
 *
 * It increments the buffer's head and reset if it overflows.
 *
 * @param tr_buf Buffer to be modified
 */
__inline__ void tr_update_head_queue(qemu_tr_buf_t* tr_buf)
{
    tr_buf->head++;
    tr_buf->head &= tr_buf->size -1;
}

//******************************************************************************
//     CYCLES, JUMP, CONDITIONAL EXECUTION AND MEMORY ACCESSES DEFINITION
//******************************************************************************
/**
 * @brief To fill the number of cycles and number of memory access
 *
 * This function fills the number of cycles and the number of memory accesses
 * decoding the instruction
 *
 * @param event Event to be decoded and filled
 *
 * @return 0 if the instruction was successfully decoded and filled or -1 if
 * the instruction was not decoded correctly
 */
__inline__ uint32_t tr_exec_analysis(hwe_cont* hwe_inst)
{
    //unsigned int cond, insn, val, op1, i, shift, rm, rs, rn, rd, sh;
    unsigned int val, op1, i, shift, rm = 0, rn = 0, rd, sh;
    uint32_t tmp = 0, tmp2 = 0;
    uint32_t ret  = 0;
    uint32_t cond = 0;
    uint32_t insn = hwe_inst->inst.body.instr;
 
    cond = insn >> 28;

    HWE_CPU_inst_set_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_NORMAL_INSTRUCTION]);
    
    // Uncoditional instructions
    if(cond == 0xf){
        if ((insn & 0x0e50ffe0) == 0x08100a00){
           // RFE
           uint32_t offset;
           uint32_t addr;

           rn = (insn >> 16) & 0xf;
           addr = cpu_single_env->regs[rn]; // TODO: Verify why the registers are not executed yet
           i = (insn >> 23) & 3;
           switch (i) {
              case 0: offset = -4; break; /* DA */
              case 1: offset =  0; break; /* IA */
              case 2: offset = -8; break; /* DB */
              case 3: offset =  4; break; /* IB */
              default: break;
           }
           addr += offset;
           HWE_CPU_inst_set_jump(hwe_inst, addr);
           HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
           HWE_CPU_inst_inc_imem(hwe_inst, 2);
           g_crt_no_cycles_instr += HWE_CPU_inst_get_cycles(hwe_inst);
           return 0;
        }else if ((insn & 0x0e000000) == 0x0a000000) {
           /* branch link and change to thumb (blx <offset>) */
           int32_t offset;
           int32_t pc = 0x0;

           pc = HWE_CPU_inst_get_pc(hwe_inst);
           val = (uint32_t)pc +4;
           /* Store pc to next instruction in r14 */
           HWE_CPU_inst_add_reg(hwe_inst);
           HWE_CPU_inst_set_regid(hwe_inst, 0, 14);
           /* Sign-extend the 24-bit offset */
           offset = (((int32_t)insn) << 8) >> 8;
           /* offset * 4 + bit24 * 2 + (thumb bit) */
           val += (offset << 2) | ((insn >> 23) & 2) | 1;
           /* pipeline offset */
           val += 4;
           HWE_CPU_inst_set_jump(hwe_inst, val);
           HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
           HWE_CPU_inst_inc_imem(hwe_inst, 2);
           g_crt_no_cycles_instr += HWE_CPU_inst_get_cycles(hwe_inst);
           return 0 ;
        } else{
           g_crt_no_cycles_instr += HWE_CPU_inst_get_cycles(hwe_inst);
           return 0;
        }
    }
    if(cond != 0xe ){
        //            event->cond = 1; // Put condition test
        if(!tr_test_cc(cond)){
            // Conditional Test
            g_crt_no_cycles_instr += HWE_CPU_inst_get_cycles(hwe_inst);
            HWE_CPU_inst_clr_exec(hwe_inst);
            return 0;
            }
        }
        if ((insn & 0x0f900000) == 0x03000000) {
            g_crt_no_cycles_instr += HWE_CPU_inst_get_cycles(hwe_inst);
            return 0;
        } else if ((insn & 0x0f900000) == 0x01000000 &&
                   (insn & 0x00000090) != 0x00000090) {
            /* miscellaneous instructions */
            op1 = (insn >> 21) & 3;
            sh = (insn >> 4) & 0xf;
            rm = insn & 0xf;
            switch (sh) {
            case 0x00: /* move program status register */
                if( op1 & 1 ){
                    /* PSR = reg */
                   HWE_CPU_inst_add_reg(hwe_inst);
                   HWE_CPU_inst_set_regid(hwe_inst, 0, CSPR_REG);
                } else {
                    /* reg = PSR */
                    rd = (insn >> 12) & 0xf;
                    HWE_CPU_inst_add_reg(hwe_inst);
                    HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                }
                break;
            case 0x1: // bx - branch exchange thumb
            {
                int32_t val;
                if (op1 == 1) {
                    /* branch/exchange thumb (bx).  */
                        //ARCH(4T);
                    val = cpu_single_env->regs[rm];
                    HWE_CPU_inst_set_jump(hwe_inst, val);
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                    HWE_CPU_inst_inc_imem(hwe_inst, 2);
                } else if (op1 == 3) {
                   /* clz */
                   rd = (insn >> 12) & 0xf;
                   HWE_CPU_inst_add_reg(hwe_inst);
                   HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                } else {
                    //    goto illegal_op;
                }
            }
                break;
            case 0x2: //bxj // branch exchange jazelle
                break;
            case 0x3: // blx // branch link/exchange thumb (blx)
            {
                int32_t val;
                /* branch link/exchange thumb (blx) */
                /* Store pc to next instruction in r14 */
                HWE_CPU_inst_add_reg(hwe_inst);
                HWE_CPU_inst_set_regid(hwe_inst, 0, 14);
                // Need to store the PC + 4 or indicate this on the trace
                val = cpu_single_env->regs[rm];
                HWE_CPU_inst_set_jump(hwe_inst, val);
                HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                HWE_CPU_inst_inc_imem(hwe_inst, 2);
           }
                break;
            /* signed multiply */
            case 0x8: case 0xa: case 0xc: case 0xe:
                HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_SIGNED_MUL]);
                if (op1 == 1)
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MUL32]);
                else
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MUL16]);
                break;
            default:
                break;
            }
        } else if (((insn & 0x0e000000) == 0 && (insn & 0x00000090) != 0x90) ||
                  ((insn & 0x0e000000) == (1 << 25))) {
            int set_cc;
            op1 = (insn >> 21) & 0xf;
            set_cc = (insn >> 20) & 1;
            /* data processing instruction */
            if ((insn & (1 << 25)) ) {
                /* immediate operand */
            } else {
                /* register */
                rm = (insn) & 0xf;
                tmp2 = cpu_single_env->regs[rm];
                if (!(insn & (1 << 4))) {
                    shift = (insn >> 7) & 0x1f;
                    if (shift){
                       HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_REGISTER_SHIFT]);
                    }
                }
            }
            if (op1 != 0x0f && op1 != 0x0d) {
                rn = (insn >> 16) & 0xf;
                tmp = cpu_single_env->regs[rn];
            }
            rd = (insn >> 12) & 0xf;

            if(rd != 15){
                HWE_CPU_inst_add_reg(hwe_inst);
                HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
            }
            switch(op1){ // TODO: Include a load register to PC here.
            case 0x00:
                tmp &= tmp2;
                if (rd == 15){
                    HWE_CPU_inst_set_jump(hwe_inst, cpu_single_env->regs[rn]);
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                    HWE_CPU_inst_inc_imem(hwe_inst, 2);
                }
                break;
            case 0x02:
                if (set_cc && rd == 15) {
                    HWE_CPU_inst_set_jump(hwe_inst, tmp);
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                    HWE_CPU_inst_inc_imem(hwe_inst, 2);
                }
                break;
            case 0x03:
               /*if(set_cc){

               }

               if (rd == 15){
                   event->inst_grp = TR_ARM_JUMP;
                   val = cpu_single_env->regs[rm];
                   event->jmp_addr = val;
                }*/
                break;
            case 0x0d:
                if (rd == 15){
                    HWE_CPU_inst_set_jump(hwe_inst,cpu_single_env->regs[rm]);
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                    HWE_CPU_inst_inc_imem(hwe_inst, 2);
                }
                break;
           case 0x0f: // SWI
                if (rd == 15){
                    HWE_CPU_inst_set_jump(hwe_inst,tmp2); // TODO: Possible bug
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                    HWE_CPU_inst_inc_imem(hwe_inst, 2);
                }
                break;
           default:
                break;
           }
       } else {
            /* other instructions */
            op1 = (insn >> 24) & 0xf;
            switch(op1) {
            case 0x0:
            case 0x1:
                /* multiplies, extra load/stores */
                sh = (insn >> 5) & 3;
                if (sh == 0) {
                    if (op1 == 0x0) {
                        rd = (insn >> 16) & 0xf;
                        rn = (insn >> 12) & 0xf;
                        op1 = (insn >> 20) & 0xf;
                        HWE_CPU_inst_add_reg(hwe_inst);
                        HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                        switch (op1) {
                        case 0: case 1: case 2: case 3: case 6:
                            /* 32 bit mul */
                            HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MUL32]);
                            if (insn & (1 << 22)) {
                                /* Subtract (mls) */
                                HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MLS]);
                            } else if (insn & (1 << 21)) {
                                /* Add */
                                HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MLA]);
                            }
                            break;
                        case 4:
                            /* 64 bit mul double accumulate (UMAAL) */
                            HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MUL64]);
                            HWE_CPU_inst_add_reg(hwe_inst);
                            HWE_CPU_inst_set_regid(hwe_inst, 1, rn);
                            break;
                        case 8: case 9: case 10: case 11:
                        case 12: case 13: case 14: case 15:
                            /* 64 bit mul: UMULL, UMLAL, SMULL, SMLAL. */
                            HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MUL64]);
                            if (insn & (1 << 21)) {
                                /* mult accumulate */
                                HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_MLA]);
                            }
                            HWE_CPU_inst_add_reg(hwe_inst);
                            HWE_CPU_inst_set_regid(hwe_inst, 1, rn);
                            break;
                        default:
                            break;
                        }
                    } else {
                        rd = (insn >> 12) & 0xf;
                        HWE_CPU_inst_add_reg(hwe_inst);
                        HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                        if (insn & (1 << 23)) {
                           // load exclusive
                           if (insn & (1 << 20)) {
                               HWE_CPU_inst_set_dmem(hwe_inst,1);
                               HWE_CPU_inst_set_excl(hwe_inst,TR_EXCL_LOAD);
                               //TODO: Include the size of access
                               //TODO: include the 64 bits access
                            // store exclusive
                            } else{
                                if(helper_test_exclusive()){
                                    HWE_CPU_inst_set_dmem(hwe_inst,1);
                                    HWE_CPU_inst_set_excl(hwe_inst,TR_EXCL_STORE);
                                    //TODO: Include the size of access.
                                    //TODO: include the 64 bits access
                                }
                            }
                        } else{
                            /* SWP instruction */
                            HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_SWP]);
                        }
                    }
                } else { // Misc load / store
                    int load = 0, j=0;
                    rn = (insn >> 16) & 0xf;
                    rd = (insn >> 12) & 0xf;
                    
                    HWE_CPU_inst_set_dmem(hwe_inst,1);

                    if (insn & (1 << 20)) {
                        load = 1;
                    } else if (!(insn & (1 << 20))) { // !load
                        if (sh & 2) { // double word
                            if (sh & 1) {
                                load = 0;
                            } else {
                                HWE_CPU_inst_add_reg(hwe_inst);
                                HWE_CPU_inst_set_regid(hwe_inst, j, rd +1);
                                j++;
                                load = 1;
                            }
                            HWE_CPU_inst_inc_dmem(hwe_inst,1);
                        }
                    }

                    if (!(insn & (1 << 24))) {
                        HWE_CPU_inst_add_reg(hwe_inst);
                        HWE_CPU_inst_set_regid(hwe_inst, j, rn);
                        j++;
                    } else if (insn & (1 << 21)) {
                        HWE_CPU_inst_add_reg(hwe_inst);
                        HWE_CPU_inst_set_regid(hwe_inst, j, rn);
                        j++;
                    }
                    if (load){
                        if (rd == 15){
                            HWE_CPU_inst_set_jump(hwe_inst,0x00); //Address is filled when receives ack
                            HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                            HWE_CPU_inst_inc_imem(hwe_inst, 2);
                        } else {
                            HWE_CPU_inst_add_reg(hwe_inst);
                            HWE_CPU_inst_set_regid(hwe_inst, j, rd);
                        }
                    }
                }
                break;
            case 0x4:
            case 0x5:
                goto tr_do_ldst;
                break;
            case 0x6:
            case 0x7:
                if (insn & (1 << 4)) {
                    rn = (insn >> 16) & 0xf;
                    rd = (insn >> 12) & 0xf;
                    switch ((insn >> 23) & 3) {
                    case 0:  /* Parallel add/subtract.  */
                        HWE_CPU_inst_add_reg(hwe_inst);
                        HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                        break;
                    case 1: /* Halfword pack.  */
                        // pkhtb, pkhbt, [us]sat, [us]sat16,
                        // sxtb16, sxtb, sxth, uxtb16, uxtb, uxth 
                        // rev 
                        HWE_CPU_inst_add_reg(hwe_inst);
                        HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                        break;
                    case 2:
                        // SMMUL, SMMLA, SMMLS
                        /* smuad, smusd, smlad, smlsd */
                        /* SDIV, UDIV */
                        HWE_CPU_inst_add_reg(hwe_inst);
                        HWE_CPU_inst_set_regid(hwe_inst, 0, rn);
                        switch ((insn >> 20) & 0x7){
                        case 0:
                        case 4:
                            if (insn & (1 << 22)) {
                                /* smlald, smlsld */
                                HWE_CPU_inst_add_reg(hwe_inst);
                                HWE_CPU_inst_set_regid(hwe_inst, 1, rd);
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case 3:
                        op1 = ((insn >> 17) & 0x38) | ((insn >> 5) & 7);
                        switch(op1){
                        case 0:
                            HWE_CPU_inst_add_reg(hwe_inst);
                            HWE_CPU_inst_set_regid(hwe_inst, 0, rn);
                            break;
                        case 0x20: case 0x24: case 0x28: case 0x2c:
                        /* Bitfield insert/clear.  */
                        case 0x12: case 0x16: case 0x1a: case 0x1e: /* sbfx */
                        case 0x32: case 0x36: case 0x3a: case 0x3e: /* ubfx */
                            HWE_CPU_inst_add_reg(hwe_inst);
                            HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                    break;
                }
            tr_do_ldst:
                /* load/store byte/word */
                i = 0;
                sh = (0xf << 20) | (0xf << 4);
                if (op1 == 0x7 && ((insn & sh) == sh)){
                    g_crt_no_cycles_instr += HWE_CPU_inst_get_cycles(hwe_inst);
                    return 0;
                }
                rn = (insn >> 16) & 0xf;
                rd = (insn >> 12) & 0xf;
                if (insn & (1 << 20)) {
                    /* load */
                    HWE_CPU_inst_add_reg(hwe_inst);
                    HWE_CPU_inst_set_regid(hwe_inst, i, rd);
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_LOAD]);
                    i++;
                } else {
                    /* store */
                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_STORE]);
                }
                if (!(insn & (1 << 24))){
                    if(rn == 15){
                         HWE_CPU_inst_set_jump(hwe_inst,0x00); //Address is filled when receives commit
                         HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                         HWE_CPU_inst_inc_imem(hwe_inst, 2);
                    }else{
                         HWE_CPU_inst_add_reg(hwe_inst);
                         HWE_CPU_inst_set_regid(hwe_inst, i, rn);
                    }
                }else if (insn & (1 << 21)){
                    if(rn == 15){
                         HWE_CPU_inst_set_jump(hwe_inst,0x00); //Address is filled when receives commit
                         HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                         HWE_CPU_inst_inc_imem(hwe_inst, 2);
                    }else{
                         HWE_CPU_inst_add_reg(hwe_inst);
                         HWE_CPU_inst_set_regid(hwe_inst, i, rn);
                    }
                }
                if (insn & (1 << 20)) {
                    if(rd == 15){
                         HWE_CPU_inst_set_jump(hwe_inst,0x00); //Address is filled when receives commit
                         HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                         HWE_CPU_inst_inc_imem(hwe_inst, 2);
                    }
                }
                HWE_CPU_inst_set_dmem(hwe_inst, 1);
            break;
            case 0x08:
            case 0x09:
            {
                /* load/store multiple words */
                int n, i, j = 0 , loaded_base = 0, user = 0, fiq_mode;
                rn = (insn >> 16) & 0xf;
                n = 0;

                if (insn & (1 << 22)) {
                    if ((insn & (1 << 15)) == 0){
                        user = 1;
                    }
                } 
                for(i = 0; i < 16; i++) {
                    if (insn & (1 << i)){
                        n++;
                    }
                }
                HWE_CPU_inst_inc_cycles(hwe_inst,
                                       n*arm_default_instr_cycle[TR_ARM_MULTI_TRANSFER_PER_REGISTER] \
                                       + arm_default_instr_cycle[(insn & (1 << 20)) ? \
                                       TR_ARM_MULTI_TRANSFER_LOAD_OP : \
                                       TR_ARM_MULTI_TRANSFER_STORE_OP]);
                HWE_CPU_inst_set_dmem(hwe_inst, n);

                fiq_mode = (cpsr_read(cpu_single_env)&CPSR_M)==ARM_CPU_MODE_FIQ ? 
                           FIQ_MODE : 0;
                for(i=0, j=0;i<16;i++) {
                    if (insn & (1 << i)) {
                        if (insn & (1 << 20)) { // load
                            // Put register list
                            if(user){
                                HWE_CPU_inst_add_reg(hwe_inst);
                                HWE_CPU_inst_set_regid(hwe_inst, j, i | USER_REGS | fiq_mode ); // ID for user 
                                j++;
                            } else if(i == rn){
                                loaded_base = 1;
                            } else {
                                if ( i == 15){
                                    HWE_CPU_inst_set_jump(hwe_inst,0x00); //Address is filled when receives commit
                                    HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                                    HWE_CPU_inst_inc_imem(hwe_inst, 2);
                                }else{
                                    HWE_CPU_inst_add_reg(hwe_inst);
                                    HWE_CPU_inst_set_regid(hwe_inst, j, i);
                                   j++;
                                }
                            }
                        }else{ // store
                        }
                    }
                }
                if (insn & (1 << 21)) {
                    HWE_CPU_inst_add_reg(hwe_inst);
                    HWE_CPU_inst_set_regid(hwe_inst, j, rn);
                    j++;
                }
                if(loaded_base){
                    HWE_CPU_inst_add_reg(hwe_inst);
                    HWE_CPU_inst_set_regid(hwe_inst, j, rn);
                }
                // IF it is a jumper put + 2 in the imem access (For a pop instruction)
            }
            break;
            case 0xa:
            case 0xb:
            {
                int32_t val, offset;
                /* branch (and link) */
                if (insn & (1 << 24)) { // (and link)
                    HWE_CPU_inst_add_reg(hwe_inst);
                    HWE_CPU_inst_set_regid(hwe_inst, 0, 14);
                }
                val = HWE_CPU_inst_get_pc(hwe_inst) + 4;
                offset = (((int32_t)insn << 8) >> 8);
                val += (offset << 2) + 4;

                HWE_CPU_inst_set_jump(hwe_inst,val); //Address is filled when receives commit
                HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_JUMP]);
                HWE_CPU_inst_inc_imem(hwe_inst, 2);
            }
                break;
            case 0xc:
            case 0xd:
            case 0xe:
                {
                uint32_t cpnum;
                /* Co-processor.  */
                HWE_CPU_inst_inc_cycles(hwe_inst,arm_default_instr_cycle[TR_ARM_COCPU]);
                // TODO: Set decoding things to show that this istruction is about cache management
                cpnum = (insn >> 8) & 0xf;                    
                switch(cpnum){
                   case 15:
                      rd = (insn >> 12) & 0xf;
                      HWE_CPU_inst_add_reg(hwe_inst);
                      HWE_CPU_inst_set_regid(hwe_inst, 0, rd);
                      break;
                   default:
                      break;

                }
                }
                break;
            default:
            break;
            }
        }
    g_crt_no_cycles_instr += HWE_CPU_inst_get_cycles(hwe_inst);
    return ret;
}

/**
 * @brief Test Conditional function
 * @param cond Conditional test
 * @return 1 if the instruction is executed 0 if it is skipped
 */
uint32_t tr_test_cc(uint32_t cond)
{
    switch (cond) {
    case 0: /* eq: Z */
        return !cpu_single_env->ZF;
    break;
    case 1: /* ne: !Z */
        return cpu_single_env->ZF;
    break;
    case 2: /* cs: C */
        return cpu_single_env->CF;
    break;
    case 3: /* cc: !C */
        return !cpu_single_env->CF;
    break;
    case 4: /* mi: N */
        return cpu_single_env->NF && (1 << 31);
    break;
    case 5: /* pl: !N */
        return !(cpu_single_env->NF & (1 << 31));
    break;
    case 6: /* vs: V */
        return (cpu_single_env->VF & (1 << 31));
    break;
    case 7: /* vc: !V */
        return !(cpu_single_env->VF & (1 << 31));
    break;
    case 8: /* hi: C && !Z */
        return (cpu_single_env->CF && cpu_single_env->ZF);
    break;
    case 9: /* ls: !C || Z */
        return (!cpu_single_env->CF || !cpu_single_env->ZF);
    break;
    case 10: /* ge: N == V -> N ^ V == 0 */
        return ((cpu_single_env->VF & (1 << 31)) ==
                (cpu_single_env->NF & (1 << 31)));
    break;
    case 11: /* lt: N != V -> N ^ V != 0 */
        return ((cpu_single_env->VF & (1 << 31)) !=
                (cpu_single_env->NF & (1 << 31)));
    break;
    case 12: /* gt: !Z && N == V */
        return ( cpu_single_env->ZF &&
           ((cpu_single_env->VF & (1 << 31)) ==
            (cpu_single_env->NF & (1 << 31))));
    break;
    case 13: /* le: Z || N != V */
        return ( !cpu_single_env->ZF ||
           ((cpu_single_env->VF & (1 << 31)) !=
            (cpu_single_env->NF & (1 << 31))));
    break;
    default:
        return 0;
    break;
    }
    return 0;
}

/**
 * @brief It increments the number of data memory access due unaligned access
 *
 */
void tr_unaligned_access(void)
{
   if(cpu_single_env->rabbits.tr_id != NULL){
      HWE_CPU_inst_inc_dmem(cpu_single_env->rabbits.tr_id,1);   
      HWE_CPU_inst_set_unalign(cpu_single_env->rabbits.tr_id);
   }
    // TODO: Is it necessary add one cycle due unaligned access ?
}


#endif /* RABBITS_TRACE_EVENT */
