
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>

#include <cfg.h>
#include <sherlock.h>
#include <components.h>
#include <stdio.h>
#include <stdlib.h>
#include <core_cpu.h>
#include <core_mem.h>
#include <plugin.h>
#include <debug.h>
#include <util_hash.h>
#include <cmd.h>
#include <gdbstub.h>



#include <delorean.h>
//#define DEBUG_REV_MEMORY
//#define DEBUG_FWD_MEMORY

//#define DEBUG_REV_REG
//#define DEBUG_FWR_REG
inst_node_t *create_inst_node(comp_type_t type, void* args);

void destroy_inst_node(inst_node_t *node);

void increase_reg_node(inst_node_t *node, uint8_t n_reg);
void dl_switch_mode(cpu_t *cpu, inst_node_t *node, uint32_t mode);

inst_node_t *add_mem_node(inst_node_t *inst);

uint32_t get_reg_last_write(inst_node_t *last_write, reg_t* new, uint8_t id);
void set_reg_next_write(inst_node_t *next_write,  reg_t* new, uint32_t value);

uint32_t cpu_reverse_exec(cpu_t* cpu);
uint32_t cpu_forward_exec(cpu_t* cpu);

void     mem_reverse_exec(mem_t *mem);
void     mem_forward_exec(mem_t* mem);

void     dev_reverse_exec(mem_t *dev);
void     dev_forward_exec(mem_t* dev);


oracle_t oracle;
struct timespec start_sys_clock, end_sys_clock;
FILE* g_fp;

size_t getPeakRSS(void)
{
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
    return (size_t)(rusage.ru_maxrss * 1024L);
}

size_t getCurrentRSS(void)
{
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return (size_t)0L;      /* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return (size_t)0L;      /* Can't read? */
    }
    fclose( fp );
    return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);
}


struct timespec timespec_diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

void output_partial_results(void)
{
    struct timespec end_sys_clock;
    struct timespec total_time;
    double elapsed_time;
    char str[32];

    clock_gettime(CLOCK_MONOTONIC, &end_sys_clock);
    total_time = timespec_diff(start_sys_clock, end_sys_clock);

    elapsed_time = (double)total_time.tv_sec + (double)(total_time.tv_nsec)/1000000000 ;
 
    printf("Total Time      : %f\n",elapsed_time);
    
    sprintf(str, "%f;", elapsed_time);
    fwrite(str, strlen(str), 1, g_fp);

    clock_gettime(CLOCK_MONOTONIC, &start_sys_clock);
}

/**
 * @brief 
 *
 * @param argc
 * @param argv[]
 *
 * @return 
 */

int delorean_pg_init(int argc, char* argv[])
{
    init_struct is;

    is.gdb_port = 0;

    parse_cmdline (argc, argv, &is);
    
    if(is.gdb_port > 0){
        gdb_start_and_wait(is.gdb_port);
    }

    // Initialize ORACLE //TODO: The oracle must be initialized in CORE 
    oracle.n_cpu     = 0;
    oracle.n_dev     = 0;
    oracle.cpu       = NULL;
    oracle.mem       = NULL;
    oracle.dev       = NULL;
    oracle.last_exec = NULL;

    // For all components
    g_fp = fopen("perf", "a");
    clock_gettime(CLOCK_MONOTONIC, &start_sys_clock);

    printf("DELOREAN - Successfully initialized\n");
    return 0;
}

/**
 * @brief 
 *
 * @param comp
 * @param vals
 * @param type
 *
 * @return 
 */
int delorean_pg_comp(void* comp, void *vals, comp_type_t type)
{
    switch(type){
    case COMP_CPU:
    {
        cpu_t *cpu = (cpu_t*)comp;
        reg_t *regs = cpu->regs;
        reg_t *banked_spsr = cpu->banked_spsr;
        reg_t *banked_r13 = cpu->banked_r13;
        reg_t *banked_r14 = cpu->banked_r14;
        reg_t *usr_regs = cpu->usr_regs;
        reg_t *fiq_regs = cpu->fiq_regs;
        uint8_t n_reg;
        
          
        //      reg_t *regs = (reg_t*)vals;
        reg_node_t *reg_node = NULL;
        int i, j;

        inst_node_t *node = NULL;

        n_reg = ARM_REGS+1; 
        node = create_inst_node(COMP_CPU, &n_reg ); // Create a RESET type
        oracle.n_cpu++;
        oracle.cpu = realloc(oracle.cpu, oracle.n_cpu*sizeof(cpu_t*));
        oracle.cpu[cpu->id] = cpu;

        if(oracle.last_exec == NULL){
            oracle.last_exec = node;
        }else{
            oracle.last_exec->next = node;
            oracle.last_exec = node;
        }

        cpu->last_exec         = node;
        node->last_exec        = node; // Reset Loop
        node->data.inst->n_reg = ARM_REGS; 
        reg_node               = node->data.inst->reg;

        reg_node->id         = REGID_CPSR;
        reg_node->value      = 0x400001C0 | ARM_CPU_MODE_SVC;
        reg_node->last_write = node;
        reg_node++;
        // Update CPU registers
        cpu->cpsr.last_write = node;

        reg_node->id         = REGID_SPSR;
        reg_node->value      = 0x0 ;
        reg_node->last_write = node;
        reg_node++;
        // Update CPU registers
        cpu->spsr.last_write = node;

        for(i = 0; i < 16; i++){
            // General Purpose Registers
            reg_node->id = i;
            reg_node->value = 0x00;
            reg_node->last_write = node;
            reg_node++;
            // Update CPU registes 
            regs[i].last_write = node;

            if (i == 15 ){
                regs[i].last_write = node;
            } else if(i > 12){ // Shared registers r13 and r14 
                 // Update CPU registers
                 banked_r13[0].last_write = node;
                 banked_r14[0].last_write = node;

                 for (j = 1; j < 6; j++){ // REG_ID from 1 to 5
                     reg_node->id = i | (j << 4);
                     reg_node->value = 0x00;
                     reg_node->last_write = node;
                     reg_node++; 
                     // Update CPU registers
                     banked_r13[j].last_write = node;
                     banked_r14[j].last_write = node;
                }
            } else if( i > 7){ // Shared register for FIQ
                reg_node->id = i | (REGID_FIQ);
                reg_node->value = 0x00;
                reg_node->last_write = node;
                reg_node++;
                // Update CPU registers
                usr_regs[i-8].last_write = node;
                fiq_regs[i-8].last_write = node;
            }
        }

        // Update CPU registers
        banked_spsr[0].last_write = node;
        for (j = 1; j < 6; j++){ // REG_ID from 1 to 5
            reg_node->id = BANK_SPSR | (j << 4);
            reg_node->value = 0x00;
            reg_node->last_write = node;
            reg_node++;
            // Update CPU registers
            banked_spsr[j].last_write = node;
        }
    }
        break;
    case COMP_MEM:
//  case COMP_DEV:
    {
        mem_t *mem = (mem_t*)comp;
        inst_node_t* first_node;
        oracle.n_mem++;
        oracle.mem = realloc(oracle.mem, oracle.n_mem*sizeof(mem_t*));
        oracle.mem[mem->id] = mem;

        first_node       = create_inst_node(COMP_MEM, NULL);
        mem->last_exec   = first_node;
        oracle.last_exec = first_node;
    }
        break;
    default:
        ERROR("Not defined component");
        return 1;
        break;
    }
    return 0;
}

/**
 * @brief 
 *
 * @return 
 */
void *delorean_pg_mem_init(void)
{
    return (void*)add_mem_node(NULL);
} 

/**
 * @brief 
 *
 * @return 
 */
bool oracle_get_trace(void)
{
    return oracle.last_exec->next == NULL ? true : false;  
}

/**
 * @brief 
 *
 * @return 
 */
inst_node_t *oracle_get_last_exec(void)
{
    return oracle.last_exec;
}

/**
 * @brief 
 *
 * @param comp
 * @param val
 * @param type
 */
void oracle_reverse_exec(gdb_param_t *param, comp_type_t *type)
{
    uint8_t  id ;
    inst_node_t* last_exec;
    uint32_t pc = 0;
//    bool     done = 0;

//    do{
        last_exec = oracle.last_exec;
        *type = last_exec->data.inst->type;

        switch (*type){
        case COMP_CPU:
        {
            cpu_t    *cpu = NULL;
            id = last_exec->data.inst->id;
            cpu = oracle.cpu[id];
            cpu->last_exec = last_exec;
            pc = cpu_reverse_exec(cpu);
            // Return Paramemters
            param->cpu.pc = pc;
            param->cpu.id = cpu->id;
//            done = 1;
        }
            break;
        case COMP_MEM:
        {
            mem_t    *mem = NULL;
            id = last_exec->data.mem->id;
            mem = oracle.mem[id];
            mem->last_exec = last_exec;
            mem_reverse_exec(mem);
            // Return Paramemters
            param->mem.id   = mem->id;
            param->mem.addr = mem->last_exec->data.mem->addr;
            param->mem.val  = mem->last_exec->data.mem->value;
//            done = 0;
        }
            break;
        case COMP_PERIPH:
        {
            mem_t    *dev = NULL;
            id = last_exec->data.dev->id;
            // At this moment it is still a memory 
            dev = oracle.mem[id]; 
//            dev = oracle.dev[id];
            dev->last_exec = last_exec;
            dev_reverse_exec(dev);
//            done = 0;
        }
            break;
        default:
            ERROR("Reverse error: Not recognized device");
            exit(0);
            break;
        }
        oracle.last_exec = last_exec->last_exec;

//  } while (!done);

//    return pc;

}

/**
 * @brief 
 *
 * @param comp
 * @param val
 * @param type
 */
void oracle_forward_exec(gdb_param_t *param, comp_type_t *type)
{
    uint8_t id;
//    bool done = 0;
    inst_node_t *next_exec;
    uint32_t pc;

//    do{
        next_exec = oracle.last_exec->next;
        *type      = next_exec->data.inst->type;


        switch (*type){
        case COMP_CPU:
        {
            cpu_t *cpu = NULL;

            id  = next_exec->data.inst->id;
            cpu = oracle.cpu[id];
            cpu->next_exec = next_exec;
            pc = cpu_forward_exec(cpu);
            // Return Parameters
            param->cpu.id = cpu->id;
            param->cpu.pc = pc; 
//            done = 1;
        }
            break;
        case COMP_MEM:
        {
             mem_t *mem = NULL;
             id  = next_exec->data.mem->id;
             mem = oracle.mem[id];
             mem->last_exec = next_exec;
             mem_forward_exec(mem);
             // Return Parameters
             param->mem.id   = mem->id;
             param->mem.addr = mem->last_exec->data.mem->addr;
             param->mem.val  = mem->last_exec->data.mem->value;
//             done = 0;
        }
            break;
        case COMP_PERIPH:
        {
            mem_t    *dev = NULL;
            id  = next_exec->data.dev->id;
//            dev = oracle.dev[id];
            dev = oracle.mem[id];
            dev->next_exec = next_exec;
            dev_forward_exec(dev);
//            done = 0;
        }
            break;
        default:
            ERROR("Forward error: Not recognized device");
            break;
        }
        oracle.last_exec = next_exec;

//    } while (!done);  // 
//  return pc;
}

/**
 * @brief 
 *
 * @param mem
 */
void mem_reverse_exec(mem_t *mem)
{
    sl_addr_t   *new;
    inst_node_t *old;
    uint32_t    addr;
    mem_data_t *mem_data, *last_write;

    old        = mem->last_exec;
    mem_data   = old->data.mem;
    addr       = mem_data->addr;
    last_write = mem_data->last_write;
    new        = mem_get_addr(addr);

    if( last_write == NULL){
        new->value = 0x00;
        new->last_write = NULL;
    } else{
        new->value      = mem_data->value;
        new->last_write = mem_data->last_write;
    }
}

/**
 * @brief 
 *
 * @param mem
 */
void mem_forward_exec(mem_t *mem )
{
    sl_addr_t* addr;
    inst_node_t * mem_node;
    mem_data_t  * mem_data;

    mem_node = mem->last_exec;
    mem_data = mem_node->data.mem;
     
    addr = mem_get_addr(mem_data->addr);

    addr->value      = mem_data->value;
    addr->last_write = mem_data->last_write;

#ifdef DEBUG_FWD_MEMORY
    if(mem_data->instr->data.dev->type == COMP_DEV)
       printf("DEVICE WRITE\t");

    if( mem_data->addr % 4){
        printf("@0x%08x => A@0x%08x = 0x%08x\n", mem_data->addr
                                               , mem_data->addr & ~0x3
                                               , addr->value);
    } else { 
        printf("@0x%08x = 0x%08x\n", mem_data->addr
                                   , addr->value); 
    }
#endif
}

/**
 * @brief 
 *
 * @param dev
 */
void dev_reverse_exec(mem_t *dev)
{

}

/**
 * @brief 
 *
 * @param dev
 */
void dev_forward_exec(mem_t* dev)
{

} 

/**
* @brief 
*
* @param cpu
*/
uint32_t cpu_reverse_exec(cpu_t* cpu )
{
    uint32_t     i, n_reg;
    uint8_t     reg_id;
    inst_node_t *inst, *last_write;
    data_t      *data;
    reg_node_t  *reg;

    if(cpu->last_exec != oracle.last_exec){
        ERROR("Inconsistent past");
    }
    
    inst = cpu->last_exec;
    data = inst->data.inst; 
    
    n_reg = data->n_reg;
    reg   = data->reg;

    // undo every wrote register
    for (i = 0; i < n_reg; i++, reg++){
        reg_id     = reg->id;
        last_write = reg->last_write;

        if(reg_id < 16){
             // Update the value and edges
             if(get_reg_last_write(last_write, &cpu->regs[reg_id], reg_id)){
                 ERROR("Register does not have a past value");
                 printf("PC = %x\treg_id = %x\n", cpu->regs[15].data, reg_id);
             }
        } else {
            switch (reg_id){
            case REGID_CPSR:
                if(get_reg_last_write(last_write, &cpu->cpsr, reg_id)){
                    ERROR("Register does not have a past value");
                    printf("PC = %x\treg_id = %x\n", cpu->regs[15].data, reg_id);
                }
                // switch mode?
                break;
            case REGID_SPSR:
                if(get_reg_last_write(last_write, &cpu->spsr, reg_id)){
                    ERROR("Register does not have a past value");
                    printf("PC = %x\treg_id = %x\n", cpu->regs[15].data, reg_id);
                }
                break;
            case USER_REGS:
                break;
            default:
                break;
            }
        }
    }

     return cpu->regs[15].data;
}


/**
 * @brief 
 *
 * @param cpu
 *
 * @return 
 */
uint32_t cpu_forward_exec(cpu_t* cpu)
{
    uint32_t    i, n_reg, value;
    uint8_t     reg_id;
    inst_node_t *inst;
    data_t      *data;
    reg_node_t  *reg;

    inst = cpu->next_exec;
    data = inst->data.inst;

    n_reg = data->n_reg;
    reg   = data->reg;

    // do every wrote register
    for (i = 0; i < n_reg; i++, reg++){
        reg_id     = reg->id;
        value      = reg->value;
        if(reg_id < 16){
            set_reg_next_write(inst, &cpu->regs[reg_id], value);
        }else{
            switch (reg_id){
            case REGID_CPSR:
                set_reg_next_write(inst, &cpu->cpsr, value);
                // switch mode?
                break;
            case REGID_SPSR:
                set_reg_next_write(inst, &cpu->spsr, value);
                break;
            case USER_REGS:
                break;
            default:
                break;
            }
        }
    }

    return cpu->regs[15].data;
}


cpu_t* oracle_get_cpu(uint32_t id)
{
    return oracle.cpu[id];
}

uint16_t oracle_get_ncpu(void)
{
   return oracle.n_cpu;
}

/**
 * @brief 
 *
 * @param inst
 * @param new
 * @param id
 *
 * @return 
 */
uint32_t get_reg_last_write(inst_node_t *last_write, reg_t* new, uint8_t id)
{
    uint8_t     n   = last_write->data.inst->n_reg;
    reg_node_t *old = last_write->data.inst->reg;
    int         i = 1;

    while(i < n && id != old->id){
        old++;
        i++;
    } 
    if(id == old->id){
        new->data       = old->value;
        new->last_write = old->last_write;
        return 0;
    }
    return -1;
} 

/**
 * @brief 
 *
 * @param next_write
 * @param new
 * @param value
 */
void set_reg_next_write(inst_node_t *next_write,  reg_t* new, uint32_t value)
{
   new->data = value;
   new->last_write = next_write; 
}

/**
 * @brief 
 *
 * @param cpu_i
 * @param regs_i
 *
 * @return 
 */
int delorean_pg_exec_cpu(void *cpu_i, void *regs_i)
{
    uint8_t     i, id;
    uint32_t    value;
    cpu_t       *cpu      = (cpu_t*)cpu_i;
    reg_t       *cpu_reg  = NULL;
    exec_reg_t  *regs     = (exec_reg_t*)regs_i;
    inst_node_t *node     = NULL;
    reg_node_t  *reg_node = NULL;

    // Create the vertex
    node = create_inst_node(COMP_CPU, &regs->nreg);
    
    // Update exec flow   
    // CPU
    node->data.inst->id       = cpu->id;
//    node->data.inst->id       = cpu->id;

//    node->last_exec      = cpu->last_exec;
//    cpu->last_exec->next = node;
//    cpu->last_exec       = node;

    // ORACLE
    node->last_exec        = oracle.last_exec;
    oracle.last_exec->next = node; 
    oracle.last_exec       = node; 

    reg_node = node->data.inst->reg;

    // Update the edges
    for(i = 0 ; i < regs->nreg; i++, reg_node++){
        id    = regs->id[i];
        value = regs->data[i];

        if(id < 16){
            cpu_reg              = &cpu->regs[id];

            reg_node->id         = id; 
            reg_node->value      = value;
            reg_node->last_write = cpu_reg->last_write;
            // Update CPU Registers
            cpu_reg->last_write  = node;
            // Verifiry the banked ones 
        } else if (regs->id[i] == REGID_CPSR){
            reg_node->id = REGID_CPSR;
            reg_node->value = regs->data[i];
            reg_node->last_write = cpu->cpsr.last_write;
            // Update CPU Registers
            cpu->cpsr.last_write = node;

            dl_switch_mode(cpu, node, regs->data[i] & CPSR_MODE);
        } else if (regs->id[i] == REGID_SPSR ){
            reg_node->id = REGID_SPSR;
            reg_node->value = regs->data[i]; 
            reg_node->last_write = cpu->spsr.last_write;
            // Update CPU Registers
            cpu->spsr.last_write = node;
        } else if(regs->id[i] & USER_REGS){
            uint8_t reg_id; 
            reg_id = regs->id[i] & 0x0F;
            reg_node->id = regs->id[i];
            reg_node->value = regs->data[i];
            if(reg_id == 13){
                reg_node->last_write = cpu->banked_r13[0].last_write;
                // Update CPU Registers
                cpu->banked_r13[0].last_write = node;
            } else if (reg_id == 14) {
                reg_node->last_write = cpu->banked_r14[0].last_write;
                // Update CPU Registers
                cpu->banked_r14[0].last_write = node;
            } else if (reg_id >= 8 && (regs->id[i] & FIQ_MODE)){
                reg_node->last_write = cpu->usr_regs[reg_id - 8].last_write;
                // Update CPU Registers
                cpu->usr_regs[reg_id - 8].last_write = node;
            } else {
                reg_node->last_write = cpu->regs[reg_id].last_write;
                // Update CPU Registers
                cpu->regs[reg_id].last_write = node;
            }
        } else {
            ERROR("Not recognized register");
            destroy_inst_node(node);
        }
    }

    return 0;
}

int delorean_pg_exit(void)
{
    struct timespec end_sys_clock;
    struct timespec total_time; 
    double elapsed_time;
    char str[32];
    clock_gettime(CLOCK_MONOTONIC, &end_sys_clock);
    total_time = timespec_diff(start_sys_clock, end_sys_clock);

    elapsed_time = (double)total_time.tv_sec + (double)(total_time.tv_nsec)/1000000000 ;

    printf("Total Time      : %f\n",elapsed_time);
    printf("Total MEM (peak): %zu Mbytes \n", getPeakRSS() );
    printf("Total MEM (cur) : %zu Mbytes \n", getCurrentRSS());
    printf("Teste DELOREAN - EXIT FUNCTION\n");

    sprintf(str, "%d;%zu;\n", oracle.n_cpu,getCurrentRSS());
    fwrite(str, strlen(str), 1, g_fp);
    fclose(g_fp);

    return 0;
}

void dl_switch_mode(cpu_t *cpu, inst_node_t *node, uint32_t mode)
{
    uint32_t old_mode;
    uint8_t i;

    reg_t *fiq_reg = NULL;
    reg_t *usr_reg = NULL;
    reg_t *regs = NULL;
    reg_node_t *reg_node = NULL;
    uint32_t bank;

    old_mode = cpu->cpsr.data & CPSR_MODE;

    if(old_mode == mode){
        return;
    }

    reg_node = &(node->data.inst->reg[node->data.inst->n_reg]); // Get last address 

    if (old_mode == ARM_CPU_MODE_FIQ || mode == ARM_CPU_MODE_FIQ){
       increase_reg_node(node, 16);
       fiq_reg = cpu->fiq_regs;
       usr_reg = cpu->usr_regs;
       regs    = cpu->regs;
    } else {
       increase_reg_node(node, 6);
    }

    if (old_mode == ARM_CPU_MODE_FIQ){
        regs = &regs[8];
        for(i = 8; i <= 12; i++, fiq_reg++, regs++, reg_node++){
            // New node
            reg_node->id = i | REGID_USR;
            reg_node->value = regs->data;
            reg_node->last_write = regs->last_write;
            // fiq_reg <- regs
            fiq_reg->last_write = node;
            // regs <- usr_regs
            regs->last_write = node;
        }
    } else if (mode == ARM_CPU_MODE_FIQ){
        for(i = 8; i <= 12; i++, usr_reg++, regs++, reg_node++){
           // New node
            reg_node->id = i | REGID_FIQ;
            reg_node->value = regs->data;
            reg_node->last_write = regs->last_write;
            // usr_reg <- regs
            usr_reg->last_write = node;
            // regs <- fiq_regs
            regs->last_write = node;
        }
    }

    // Update regs for old mode
    bank = cpu_bank(old_mode & CPSR_MODE);

    // Init REG NODE function
    reg_node->id = 13 | bank;
    reg_node->value = regs->data;
    reg_node->last_write = cpu->banked_r13[bank].last_write;
    reg_node++;

    cpu->banked_r13[bank].last_write = node; 

    reg_node->id = 14 | bank;
    reg_node->value = regs->data;
    reg_node->last_write = cpu->banked_r14[bank].last_write;
    reg_node++;

    cpu->banked_r14[bank].last_write = node; 

    reg_node->id = BANK_SPSR | bank << 4;
    reg_node->value = regs->data;
    reg_node->last_write = cpu->banked_spsr[bank].last_write;
    reg_node++;

    cpu->banked_spsr[bank].last_write = node; 

    // Update regs for new mode
    bank = cpu_bank(mode & CPSR_MODE);

    reg_node->id = 13 | bank;
    reg_node->value = regs->data;
    reg_node->last_write = cpu->banked_r13[bank].last_write;
    reg_node++;

    cpu->banked_r13[bank].last_write = node; 

    reg_node->id = 14 | bank;
    reg_node->value = regs->data;
    reg_node->last_write = cpu->banked_r13[bank].last_write;
    reg_node++;

    cpu->banked_r14[bank].last_write = node;

    reg_node->id = BANK_SPSR | bank << 4;
    reg_node->value = regs->data;
    reg_node->last_write = cpu->banked_spsr[bank].last_write;
    reg_node++;

    cpu->banked_spsr[bank].last_write = node; 

    // Put this info in the paper, the changing mode 
    // Even with equal values needs to maintain the link

}

/**
 * @brief 
 *
 * @param n_reg
 *
 * @return 
 */
inst_node_t *create_inst_node(comp_type_t type, void* args)
{
    inst_node_t *node = NULL;
    node = malloc(sizeof(inst_node_t));
    node->last_exec = NULL;
    node->next = NULL;

    switch(type){
    case COMP_CPU:
    {
        data_t *data;
        node->data.inst = malloc(sizeof(data_t));

        data = node->data.inst; 
        data->n_reg = *(uint8_t*)args;
        data->reg   = malloc(data->n_reg*sizeof(reg_node_t));
        data->n_mem = 0;
        data->mem   = NULL;
        data->type  = COMP_CPU;
    }
        break;
    case COMP_MEM:
        node->data.mem = malloc(sizeof(mem_data_t));
        node->data.mem->type = COMP_MEM;
        break;
    case COMP_PERIPH:
        node->data.dev = malloc(sizeof(dev_data_t));
        node->data.dev->type = COMP_PERIPH;
        break;
    default:
        ERROR("Component not recognized");
        break;
    }

    return node;
}

/**
 * @brief 
 *
 * @param node
 */
void destroy_inst_node(inst_node_t *node)
{
    switch(node->data.inst->type){
       case COMP_CPU:
          free(node->data.inst->reg);
          break;
       default:
          break;
    }
    free(node->data.inst);
    free(node);
}

/**
 * @brief  Used when there are mode registers to take into account
 *
 * @param node
 * @param n_reg
 */
void increase_reg_node(inst_node_t *node, uint8_t n_reg)
{
    node->data.inst->n_reg += n_reg;
    node->data.inst->reg = realloc(node->data.inst->reg, node->data.inst->n_reg*sizeof(reg_node_t));
}

/**
 * @brief 
 *
 * @param inst
 *
 * @return 
 */
inst_node_t *add_mem_node(inst_node_t *inst)
{
    uint32_t     n_mem    = ++inst->data.inst->n_mem;
    inst_node_t  **mem;
    inst_node_t  *mem_node = NULL;

    mem_node = create_inst_node(COMP_MEM, NULL);

    if(inst->data.inst->type == COMP_CPU){
        mem = inst->data.inst->mem;
        mem = realloc(mem, n_mem*sizeof(inst_node_t*));

        inst->data.inst->mem = mem;
        mem[n_mem-1] = mem_node;
        mem_node->data.mem->instr = inst;
    }
    
    return mem_node;
}

/**
 * @brief 
 *
 * @param mem_i
 * @param addr_i
 * @param inst_i
 * @param addr
 *
 * @return 
 */
int delorean_pg_exec_mem( void* mem_i, void *addr_i, void* inst_i, uint8_t type, uint32_t addr, void* e)
{
    sl_addr_t      *data     = (sl_addr_t*)addr_i;
    inst_node_t *inst     = (inst_node_t*)inst_i;
    mem_t       *mem      = (mem_t*)mem_i;
    inst_node_t *mem_node = add_mem_node(inst); // Create and link it with instruction
    mem_data_t  *mem_data = mem_node->data.mem;

    //Update node
    mem_data->addr  = addr;
    mem_data->value = data->value;
    mem_data->id    = mem->id;

    // Update edges
    mem_data->last_write = data->last_write;
    data->last_write     = mem_node->data.mem;

    //ORACLE
    mem_node->last_exec = oracle.last_exec;
    oracle.last_exec->next = mem_node;
    oracle.last_exec = mem_node;


    return 0;    
}


int delorean_pg_exec_dev(void* dev)
{
    inst_node_t* node;

    // DEVICE
    node = create_inst_node(COMP_PERIPH, NULL);
    node->data.dev->id = ((mem_t*)dev)->id;

    // ORACLE
    node->last_exec = oracle.last_exec;
    oracle.last_exec->next = node; 
    oracle.last_exec = node;

    return 0;

}

int delorean_pg_exec_cache(void* cache, uint8_t type, uint32_t addr, void* e)
{
    return 0;
}

SL_PLUGIN_INIT(delorean_pg_init)
SL_PLUGIN_COMP(delorean_pg_comp)
SL_PLUGIN_EXEC_CPU(delorean_pg_exec_cpu)
SL_PLUGIN_EXEC_CACHE(delorean_pg_exec_cache)
SL_PLUGIN_EXEC_MEM(delorean_pg_exec_mem)
SL_PLUGIN_EXEC_DEV(delorean_pg_exec_dev)
SL_PLUGIN_EXIT(delorean_pg_exit)
SL_PLUGIN_MEM_INIT(delorean_pg_mem_init)
 

