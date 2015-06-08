#include <sherlock.h>
#include <gdbstub.h>
#include <debug.h>
#include <core_cpu.h>
#include <delorean.h>

static void cpu_go_inst(event_t*);
//static void cpu_ret_inst(event_t*e);
static void cpu_go_st_mem(event_t*);
//static void cpu_ret_st_mem(event_t*e);
static void cpu_go_ld_mem(event_t*);

//uint32_t cpu_bank(uint32_t mode);
void dump_regs(struct comp_data_t* cd, uint32_t pc);

void dump_regs(struct comp_data_t* cd, uint32_t pc)
{

    int i;
    static int dump = 0;
    dump++;
    static FILE* fdump = NULL;

    if(fdump == NULL){
        fdump = fopen("sl_reg_dump", "w+");
    }

    if( dump == 1){
        dump = 0;
        fprintf(fdump, "pc = 0x%08x\n", pc);

        fprintf(fdump, "cspr = 0x%08x\n", cd->comp.cpu->cpsr.data);
        fprintf(fdump, "spsr = 0x%08x\n", cd->comp.cpu->spsr.data);

        for(i = 0; i < 15; i++){ // PC excluded
            fprintf(fdump, "reg[%d] = 0x%08x\n", i, cd->comp.cpu->regs[i].data);
        }
        for(i = 0; i < 6; i++){
            fprintf(fdump, "banked_r13[%d]  = 0x%08x\n", i, cd->comp.cpu->banked_r13[i].data);
            fprintf(fdump, "banked_r14[%d]  = 0x%08x\n", i, cd->comp.cpu->banked_r14[i].data);
            fprintf(fdump, "banked_spsr[%d] = 0x%08x\n", i, cd->comp.cpu->banked_spsr[i].data);
        }
        for(i = 0; i < 5; i++){
            fprintf(fdump, "fiq_regs[%d] = 0x%08x\n", i, cd->comp.cpu->fiq_regs[i].data);
        }
        for(i = 0; i < 5; i++){
            fprintf(fdump, "usr_regs[%d] = 0x%08x\n", i, cd->comp.cpu->usr_regs[i].data);
        }
        fprintf(fdump, "\n");
    }
}


/**
 * @brief
 *
 * @param e
 */
void cpu_init (event_t *e)
{
//    struct event_data_t *ed = event_data(e);
    struct comp_data_t *cd = comp_data(event_component(e));
    hwe_cont *hwe = event_content(e);


    switch(hwe->common.head.type)
    {
    case HWE_MEM32:
        if (hwe->mem.body.mem32.inst) {
           /*
            * don't care about instruction fetches
            */
            set_cb_go (e, cb_nop);
            set_cb_ret(e, cb_nop);
            put_infifo_go(&cd->fifo, e); // TODO: Is not put_infifo_go(&cd->fifo, e)?
        } else{
            switch(hwe->mem.body.mem32.access) {
            case HWE_MEM_STORE:
               set_cb_go (e, cpu_go_st_mem);
               set_cb_ret(e, cb_nop);
               put_infifo_go(&cd->fifo, e); // TODO: Is not put_infifo_go(&cd->fifo, e)?
               break;
            case HWE_MEM_LOAD:
               set_cb_go (e, cpu_go_ld_mem);
               set_cb_ret(e, cb_nop);
               put_infifo_go(&cd->fifo, e); // TODO: Is not put_infifo_go(&cd->fifo, e)?
               // TODO: put_infifo_ret(&cd->fifo, e); // TODO: Is not put_infifo_go(&cd->fifo, e)?
               break;
            default:
               ERROR("Event not defined");
               exit(1);
               break;
            }
        }
        break;
    case HWE_INST32:
        #ifdef GDB_ENABLED
        gdb_process(cd->comp.cpu, e->hwe->inst.body.pc);
        #endif
        set_cb_go (e, cpu_go_inst);
        set_cb_ret(e, cb_nop);
        put_infifo_go(&cd->fifo, e);
        break;
    case HWE_CPU_IO:
       set_cb_go (e, cb_nop);
       set_cb_ret(e, cb_nop);
       break;
    default:
        ERROR("Event not defined");
        break;
    }
}


/**
 * @brief 
 *
 * @param e
 */
/*static void cpu_ret_inst(event_t*e)
{
    // TODO: Put this routine in hwetrace_read.c instead 

}*/


/*
 *   go phase of instruction
 *  
 */
static void cpu_go_inst(event_t*e)
{
    hwe_cont *hwe = event_content(e);
    struct event_data_t *ed = event_data(e);
    struct comp_data_t  *cd = comp_data(event_component(e));
    int i = 0;

    if (hwe->inst.body.exec){
        for(i = 0, ed->regs.nreg = hwe->inst.body.nregs; i < hwe->inst.body.nregs; i++){
            ed->regs.id[i] = hwe->inst.reg_id[i];
            ed->regs.data[i] = hwe->inst.reg_data[i];
            // Alternativelly call cpu_exec to avoid so many "for"s
            // Call plugin also when it is configured to this granularity 
        }
        // Update PC
        ed->regs.nreg++;
        ed->regs.id[i] = 15;
        if(hwe->inst.body.jump){
            ed->regs.data[i] = hwe->inst.jump_pc;
        }else {
            ed->regs.data[i] = hwe->inst.body.pc + 4; 
        }
        if(hwe->inst.body.dmem){
           // Next steps and wait for other events 
        }
    }else { // Not executed instruction (update PC)
        ed->regs.nreg = 1;
        ed->regs.id[0] = 15;
        ed->regs.data[0] = hwe->inst.body.pc + 4;
    }

    // TODO: Put those function inside 
    cpu_exec(cd->comp.cpu, &ed->regs);

    pg_exec_cpu(cd->comp.cpu, &ed->regs);

    // This pointer will be used by other instructions
    e->data.inst_node = oracle_get_last_exec();
    
    //  printf("[%d.%d] GO\n", (uint32_t)e->hwe->common.id.devid, (uint32_t)e->hwe->common.id.index);

    // Dump registers
    // dump_regs(cd, e->hwe->inst.body.pc);
}

static void cpu_go_st_mem(event_t*e)
{    
    struct event_data_t *rd = event_data(event_ref(e,0));
    struct event_data_t *ed = event_data(e);
    
    ed->inst_node = rd->inst_node;
}

/*static void cpu_ret_st_mem(event_t*e)
{
    printf("[%d.%d] RET\n", (uint32_t)e->hwe->common.id.devid, (uint32_t)e->hwe->common.id.index);
}*/


static void cpu_go_ld_mem(event_t*e)
{

}

void cpu_exec(cpu_t *cpu, exec_reg_t *regs)
{
    int i;

    for(i = 0; i < regs->nreg; i++){
        if (regs->id[i] < N_GPREG){
            cpu->regs[regs->id[i]].data = regs->data[i];
        } else if (regs->id[i] == REGID_CPSR){
            switch_mode(cpu, regs->data[i] & 0x1f);
            cpu->cpsr.data = regs->data[i];
        } else if (regs->id[i] == REGID_SPSR ){ 
            cpu->spsr.data = regs->data[i];
        } else if(regs->id[i] & USER_REGS){
            uint8_t reg_id;
            reg_id = regs->id[i] & 0x0F; 
            if(reg_id == 13){
                cpu->banked_r13[0].data = regs->data[i];
            }else if (reg_id == 14){
                cpu->banked_r14[0].data = regs->data[i];
            } else if (reg_id >= 8 && (regs->id[i] & FIQ_MODE)){
                cpu->usr_regs[reg_id - 8].data = regs->data[i];
            } else {
                cpu->regs[reg_id].data = regs->data[i];
            }
//            ERROR("USER_REGS");
        }else{
            ERROR("Not recognized register");
        }
    } 
}

//==========================================
/**
 * @brief 
 *
 * @param e
 *
 * @return 
 */
cpu_t* cpu_create(event_t* e)
{    
    hwe_cont *hwe = event_content(e);

    cpu_t* cpu;
    // Create Registers
    cpu = calloc(1, sizeof(cpu_t));
    cpu->cpsr.data = 0x400001C0 | ARM_CPU_MODE_SVC;
    // Set ID
    cpu->id = hwe->info.common.id.devid;

    return cpu;
}

/**
 * @brief 
 *
 * @param cpu
 */
void cpu_destroy(cpu_t* cpu)
{
    free(cpu);
}

/**
 * @brief 
 *
 * @param mode
 *
 * @return 
 */
uint32_t cpu_bank(uint32_t mode){
    switch (mode) {
    case ARM_CPU_MODE_USR:
    case ARM_CPU_MODE_SYS:
        return 0;
    case ARM_CPU_MODE_SVC:
        return 1;
    case ARM_CPU_MODE_ABT:
        return 2;
    case ARM_CPU_MODE_UND:
        return 3;
    case ARM_CPU_MODE_IRQ:
        return 4;
    case ARM_CPU_MODE_FIQ:
        return 5;
    }
    ERROR("Not supported mode");
    return -1;
}

void switch_mode(cpu_t *cpu, uint32_t mode)
{
    uint32_t old_mode, bank;

    old_mode = cpu->cpsr.data & CPSR_MODE;

    if (mode == old_mode){
        return;
    }

    if(old_mode == ARM_CPU_MODE_FIQ ){
        // Copy just data, not the metadata // modify
        memcpy(cpu->fiq_regs, cpu->regs + 8, 5 * sizeof(reg_t));
        memcpy(cpu->regs + 8, cpu->usr_regs, 5 * sizeof(reg_t));
    } else if (mode == ARM_CPU_MODE_FIQ){
        memcpy(cpu->usr_regs, cpu->regs + 8, 5 * sizeof(reg_t));
        memcpy(cpu->regs + 8, cpu->fiq_regs, 5 * sizeof(reg_t));
    }
 
    bank = cpu_bank(old_mode & CPSR_MODE);
    cpu->banked_r13[bank].data = cpu->regs[13].data;
    cpu->banked_r14[bank].data = cpu->regs[14].data;
    cpu->banked_spsr[bank].data = cpu->spsr.data;

    bank = cpu_bank(mode & CPSR_MODE);
    cpu->regs[13].data = cpu->banked_r13[bank].data;
    cpu->regs[14].data = cpu->banked_r14[bank].data;
    cpu->spsr = cpu->banked_spsr[bank];
}

/**
* @brief 
*
* @param pc
* @param cpu
*/
void gdb_process(cpu_t* cpu, unsigned long pc)
{
//   do{
//      while(gdb_exec_direction()){ // FORWARD
//           cpu_reverse_exec(cpu);
//      }
//
    gdb_param_t gdb_param;

    gdb_param.cpu.id = cpu->id;
    gdb_param.cpu.pc = pc;

    gdb_verify(COMP_CPU, &gdb_param);

//      if (gdb_condition (pc)){
//          gdb_loop(-1,0,0,cpu);
//      } 
//   }while(gdb_exec_direction())
}

