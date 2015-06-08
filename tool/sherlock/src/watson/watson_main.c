#include <cfg.h>
#include <sherlock.h>
#include <components.h>
#include <gdbstub.h>
#include <debug.h>
#include <core_cpu.h>
#include <core_cache.h>
#include <errno.h>
#include <time.h>
#include <sys/resource.h>

#include <watson.h>

#include <libdebughelper.h>
#include <src_lines.h>

#define STATE_INIT      0
#define STATE_ANALYSE   1
#define STATE_MODIFY    2
#define STATE_WRITEBACK 3
#define STATE_PROBLEM   4

#define WRITETHROUGH 1
#define WRITEBACK    2
#define LOCK_SIZE 51
uint32_t lock[LOCK_SIZE] = {
0x1142280, 
0x115b760,
0x11424e0,
0x1159560,
0x103c080,
0x103c060,
0x103bfc0,
0x1175318,
0x11483e0,
0x1148460,
0x1149500,
0x11494c0,
0x1149580,
0x103c100,
0x11484e0,
0x11485e0,
0x1175398,
0x11789b8,
0x117a6b8,
0x117a738,
0x103bfa0,
0x1142660,
0x103bf60,
0x117b618,
0x117a7b8,
0x117b698,
0x117c4f8,
0x117c578,
0x103dc60,
0x103dc80,
0x103dd60,
0x103dd80,
0x103de60,
0x103de80,
0x103e060,
0x117b720,
0x117b700,
0x103e080,
0x11427e0,
0x103e260,
0x103e280,
0x103e860,
0x103e960,
0x103e880,
0x1159680,
0x1159700,
0x103e980,
0x1159600,
0x1142360,
0x11483a0,
};





typedef enum coherence_event_t{
    EVENT_WRITE_REQ,
    EVENT_READ_REQ,
    EVENT_ALLOC_REQ,
    EVENT_MODIFY_ACK,
    EVENT_WRITE_ACK,
    EVENT_READ_ACK,
} coherence_event_t;

typedef struct problem{
    uint32_t total;
    uint32_t mod_read_ack;
    uint32_t mod_read_req;
    uint32_t write_read_ack;
    uint32_t write_flush;
}problem_t;

typedef struct warning{
    uint32_t total;
    uint32_t mod_read_alloc;
    uint32_t mod_write;

}warning_t;

typedef struct undefined{
    uint32_t total;
}undefined_t;


typedef struct oracle_t{
    uint64_t      sys_order;    
    uint8_t       n_comp;
    uint8_t       write_policy;
    problem_t     problem;
    warning_t     warning;
    undefined_t   undefined;
    uint8_t       (*dfa_coherence)(uint32_t addr, void* comp, void* comp_event,  coherence_event_t event);
    cache_t       *comp[256];
//    union components {
//        cpu_t *cpu;
//        cache_t *cache;
//        mem_t *mem;
//    } *comp[256];
//    oracle_comp_t* comp[256];
}oracle_t;

oracle_t oracle;
struct timespec start_sys_clock, end_sys_clock;

debug_helper_t *dbg_linux = NULL;

reader_t *watson_pending_read_ack(cache_t* cache, uint32_t addr);
reader_t *watson_pending_mod_ack(cache_t* cache, uint32_t addr);

void watson_cache_g_order(cache_t* cache, uint64_t order);

void watson_event_to_reader(event_t *e, reader_t* reader);
void watson_event_to_writer(event_t *e, writer_t* writer);

void PRINT_READER(reader_t *reader);
void PRINT_WRITER(writer_t *writer);
void PRINT_MODIFIER(modifier_t *writer);
void PRINT_SRC_READER(reader_t *reader);
void PRINT_SRC_WRITER(writer_t *writer);
void PRINT_SRC_MODIFIER(modifier_t *writer);

void PRINT_ADDR(uint32_t addr);

uint8_t watson_dfa_writethrough(uint32_t addr, void* comp, void* comp_event,  coherence_event_t event);
uint8_t watson_dfa_writeback   (uint32_t addr, void* comp, void* comp_event,  coherence_event_t event);


typedef struct
{
    char               elf_filename[512];
    uint8_t            write_policy;
} init_struct;

enum
{
        CMDLINE_OPTION_elf_filename,
        CMDLINE_OPTION_write_policy,
};

typedef struct
{
    const char      *name;
    int             flags;
    int             index;
} cmdline_option;

#define HAS_ARG             0x0001

init_struct is = {
    .write_policy = -1,
};

const cmdline_option cmdline_options[] =
{
    {"elf",     HAS_ARG, CMDLINE_OPTION_elf_filename},
    {"write",   HAS_ARG, CMDLINE_OPTION_write_policy},
    {"NULL"},

};

void parse_cmdline (int argc, char **argv, init_struct *is)
{
    int                 optind;
    const char          *r, *optarg;

    optind = 0;
    for (; ;)
    {
        if (optind >= argc)
            break;
        r = argv[optind];

        const cmdline_option *popt;
        optind++;
        /* Treat --foo the same as -foo.  */
        if (r[1] == '-')
            r++;
        popt = cmdline_options;
        for (;;)
        {
            if (!popt->name)
            {
                fprintf (stderr, "%s: invalid option -- '%s'\n", argv[0], r);
                exit (1);
            }
            if (!strcmp (popt->name, r + 1))
                break;
            popt++;
        }
        if (popt->flags & HAS_ARG)
        {
            if (optind >= argc)
            {
                fprintf (stderr, "%s: option '%s' requires an argument\n",
                        argv[0], r);
                exit (1);
            }
            optarg = argv[optind++];
        }
        else
           optarg = NULL;


        switch (popt->index){ 
        case CMDLINE_OPTION_elf_filename:
            strcpy(is->elf_filename, optarg);
            break;
        case CMDLINE_OPTION_write_policy:
            if(!strcmp("writetrough",optarg)){
                is->write_policy = WRITETHROUGH;
            }else if (!strcmp("writeback",optarg)){
                is->write_policy = WRITEBACK;
            } else{
                is->write_policy = -1;
            }
            break;

        }
    }
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

#define ASSERT_MOD_READ_ORDER(a,_mod,_read) if(((modifier_t*)_mod)->pos_order > ((reader_t*)_read)->pre_order){\
                              PRINT_ADDR(a);\
                              PRINT_MODIFIER(_mod);\
                              PRINT_READER(_read);\
                              PRINT_SRC_MODIFIER(_mod);\
                              PRINT_SRC_READER(_read);\
                              oracle.undefined.total++;\
                              ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;}

#define ASSERT_MOD_WRIT_ORDER(a,_mod,_writ) if(((modifier_t*)_mod)->pos_order > ((writer_t*)_writ)->pre_order){\
                              PRINT_ADDR(a);\
                              PRINT_MODIFIER(_mod);\
                              PRINT_WRITER(_writ);\
                              PRINT_SRC_MODIFIER(_mod);\
                              PRINT_SRC_WRITER(_writ);\
                              oracle.undefined.total++;\
                              ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;/*(exit(1);*/}

#define ASSERT_READ_MOD_ORDER(a,_read,_mod) if(((reader_t*)_read)->pos_order > ((modifier_t*)_mod)->pre_order) {\
                              PRINT_ADDR(a);\
                              PRINT_READER(_read);\
                              PRINT_MODIFIER(_mod);\
                              PRINT_SRC_READER(_read);\
                              PRINT_SRC_MODIFIER(_mod);\
                              oracle.undefined.total++;\
                              ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;/*(exit(1);*/}

#define ASSERT_MOD_MOD_ORDER(a,_mod_1,_mod_2) if(((modifier_t*)_mod_1)->id != ((modifier_t*)_mod_2)->id){\
                              if(((modifier_t*)_mod_1)->pos_order > ((modifier_t*)_mod_2)->pre_order) {\
                                  PRINT_ADDR(a);\
                                  PRINT_MODIFIER(_mod_1);\
                                  PRINT_MODIFIER(_mod_2);\
                                  PRINT_SRC_MODIFIER(_mod_1);\
                                  PRINT_SRC_MODIFIER(_mod_2);\
                                  oracle.undefined.total++;\
                                  ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;/*(exit(1);*/}}

#define ASSERT_WRIT_MOD_ORDER(_a,_writ,_mod)/* if(((writer_t*)_writ)->id != ((modifier_t*)_mod)->id){*/\
                              if(((writer_t*)_writ)->pos_order > ((modifier_t*)_mod)->pre_order) {\
                                  PRINT_ADDR(_a);\
                                  PRINT_WRITER(_writ);\
                                  PRINT_MODIFIER(_mod);\
                                  PRINT_SRC_WRITER(_writ);\
                                  PRINT_SRC_MODIFIER(_mod);\
                                  oracle.undefined.total++;\
                                  ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;/*(exit(1);*/}/*}*/

#define ASSERT_WRIT_READ_ORDER(a,_writ,_read) if(((writer_t*)_writ)->pos_order > ((reader_t*)_read)->pre_order) {\
                              PRINT_ADDR(a);\
                              PRINT_WRITER(_writ);\
                              PRINT_READER(_read);\
                              PRINT_SRC_WRITER(_writ);\
                              PRINT_SRC_READER(_read);\
                              oracle.undefined.total++;\
                              ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;/*(exit(1);*/}

#define ASSERT_WRIT_WRIT_ORDER(a,_writ1,_writ2) if(((writer_t*)_writ1)->pos_order > ((modifier_t*)_writ2)->pre_order) {\
                              PRINT_ADDR(a);\
                              PRINT_WRITER(_writ1);\
                              PRINT_WRITER(_writ2);\
                              PRINT_SRC_WRITER(_writ1);\
                              PRINT_SRC_WRITER(_writ2);\
                              oracle.undefined.total++;\
                              ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;/*(exit(1);*/}


#define ASSERT_WRIT_READ_MOD_ORDER(a,_writ,_read,_mod) if(((reader_t*)_read)->pos_order > ((modifier_t*)_mod)->pre_order) {\
                              PRINT_ADDR(a);\
                              PRINT_WRITER(_writ);\
                              PRINT_READER(_read);\
                              PRINT_MODIFIER(_mod);\
                              PRINT_SRC_WRITER(_writ);\
                              PRINT_SRC_READER(_read);\
                              PRINT_SRC_MODIFIER(_mod);\
                              oracle.undefined.total++;\
                              ERROR(ANSI_COLOR_YELLOW "Undefined Order" ANSI_COLOR_RESET);\
                              *state=STATE_PROBLEM;\
                              break;/*(exit(1);*/}




//#define STACK_START 0x1038d00
//#define STACK_END   (STACK_START + 0x00010000)

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"


//#define DEBUG_WARNINGS
#define MAX_ERRORS 1000000
#define MAX_WARNS  1000000


#define PRINT_PROBLEM(a,w,m,r) PRINT_ADDR(a);\
                               PRINT_WRITER(w);\
                               PRINT_MODIFIER(m);\
                               PRINT_READER(r);\
                               PRINT_SRC_WRITER(w);\
                               PRINT_SRC_MODIFIER(m);\
                               PRINT_SRC_READER(r);\
                               oracle.problem.total++;\
                               if(oracle.problem.total > MAX_ERRORS) exit(1);

#ifdef DEBUG_WARNINGS
#define PRINT_WARNNING(str,a,w,m,r) printf(ANSI_COLOR_YELLOW "[%s] 0x%08x\n"ANSI_COLOR_RESET, str, a);\
                                    PRINT_ADDR(a);\
                                    PRINT_WRITER(w);\
                                    PRINT_MODIFIER(m);\
                                    PRINT_READER(r);\
                                    PRINT_SRC_WRITER(w);\
                                    PRINT_SRC_MODIFIER(m);\
                                    PRINT_SRC_READER(r);\
                                    if(oracle.warning.total > MAX_WARNS) exit(1); 
#else
#define PRINT_WARNNING(str,a,w,m,r)
#endif

#define ADDR_DBG 0x001abfbb
//#define ADDR_DBG 0

#define PRINT_ADDR_DBG(_str,_addr,_handler) if( ADDR_DBG == _addr)\
                           printf("%s\taddr 0x%08x [%d] [%ld] size %02d of %02d\tpc 0x%08x insn 0x%08x\n",_str,_addr,\
                                   ((cache_t*)cache)->id,oracle.sys_order, i,size, _handler->pc, hwe_ref->inst.body.instr);
                    


/* ==========================================================================
 *                            PLUGIN CONFIGURATION 
 * ========================================================================== */

/**
 * @brief 
 *
 * @param argc
 * @param argv[]
 * @param o
 *
 * @return 
 */
int watson_pg_init(int argc, char* argv[] /* oracle_t* o*/)
{
    /*    oracle = o;*/
//  line_nfo_t *line; 
//    insn_desc_t instr = {0xc000e118,0};

    printf("Test\n");
    parse_cmdline(argc, argv, &is);

    printf("Reading DWARF...");   
    fflush(stdout);

    if(is.elf_filename == NULL){
       ERROR("--elf option is not filled, then no source code information will be given");
    }else{
       dbg_linux = dh_init(is.elf_filename);
    }

//    if(is.write_policy == -1 || is.write_policy == WRITETHROUGH){
//        oracle.write_policy  = WRITETHROUGH;
//        oracle.dfa_coherence = watson_dfa_writethrough;
//    } else{ 
        oracle.write_policy  = WRITEBACK;
        oracle.dfa_coherence = watson_dfa_writeback;
//    }

    printf("done\n");    

    oracle.n_comp=0;
    oracle.sys_order = 0;
    memset(&oracle.problem,0, sizeof(problem_t));
    memset(&oracle.warning,0, sizeof(warning_t));
    memset(&oracle.undefined,0, sizeof(undefined_t));
//  line = dh_addr2line(dbg_linux,&instr);

//    printf("file %s,  line %d\n", dbg_linux->dbg->src_tabs->files[line->fileno], line->lineno);


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
int watson_pg_comp(void* comp, void *vals, comp_type_t type)
{
    if(comp == NULL){
       ERROR("WATSON: null component initialized");
       printf("n_comp %d\n", oracle.n_comp);

       oracle.n_comp++;
//       oracle.comp = realloc(oracle.comp, (oracle.n_comp)*sizeof(oracle_comp_t*));
       return 1;
    }
 
    if(oracle.n_comp != ((common_comp_t*)comp)->id){ 
        printf("n_comp %d comp id %d\n", oracle.n_comp ,((common_comp_t*)comp)->id);
        ERROR("WATSON: Device not registered in ORACLE");    
        exit(1);
    }

//    oracle.comp = realloc(oracle.comp, (oracle.n_comp)*sizeof(oracle_comp_t*));
//    printf("Comp init: [%d]  \n", ((common_comp_t*)comp)->id);

    switch(type){
    case COMP_CPU:
        oracle.comp[oracle.n_comp] = NULL;
        // Initialize the processor // XXX: Probably it should be not necessary
        break;
    case COMP_CACHE:
    {
        cache_data_t *cache_data = &((cache_t*)comp)->cache_data;
        
        cache_data->g_order              = 0;
        cache_data->n_pend_read          = 0;
        cache_data->n_pend_mod           = 0;

        cache_data->stats.max_n_pend_mod = 0;
        cache_data->stats.max_n_pend_read= 0;
        cache_data->stats.false_positive = 0;

        oracle.comp[oracle.n_comp] = comp;
    }
        break;
    case COMP_MEM:
        oracle.comp[oracle.n_comp] = NULL;
        // Initialize the memory
        // Get the shared memory 
        break;
    case COMP_PERIPH:
        oracle.comp[oracle.n_comp] = NULL;
        // Initialize the peripheral
        break;
    default:
        ERROR("Not defined component");
        break;
    }
    oracle.n_comp++;

    return 0;
}

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


/**
 * @brief 
 *
 * @return 
 */
int watson_pg_exit(void)
{   

    struct timespec end_sys_clock;
    struct timespec total_time; 
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &end_sys_clock);
    total_time = timespec_diff(start_sys_clock, end_sys_clock);

    elapsed_time = (double)total_time.tv_sec + (double)(total_time.tv_nsec)/1000000000 ;

    printf("WATSON - EXIT FUNCTION\n");

    int i, false_positives = 0;
    cache_data_t *cache_data = &oracle.comp[6]->cache_data;

    for(i = 0; i > oracle.n_comp; i++){
        if (oracle.comp[i] != NULL){
            cache_data = &oracle.comp[i]->cache_data;
            printf("[%d] n_pend %d G %ld MP %d\n", i, cache_data->n_pend_read, cache_data->g_order,
                    cache_data->stats.max_n_pend_read);

            oracle.sys_order++;
            watson_cache_g_order(oracle.comp[i], oracle.sys_order);

            false_positives += cache_data->stats.false_positive;

        }
    }
    
    printf("\nFalse Positives            : %d\n", oracle.undefined.total);
    printf(ANSI_COLOR_RED "Number of problems         : %d\n" ANSI_COLOR_RESET, oracle.problem.total);
    printf(ANSI_COLOR_RED "\tMOD   -> READ_ACK  : %d\n" ANSI_COLOR_RESET, oracle.problem.mod_read_ack);
    printf(ANSI_COLOR_RED "\tMOD   -> READ_REQ  : %d\n" ANSI_COLOR_RESET, oracle.problem.mod_read_req);
    printf(ANSI_COLOR_RED "\tWRITE -> READ_ACK  : %d\n" ANSI_COLOR_RESET, oracle.problem.write_read_ack);
    printf(ANSI_COLOR_RED "\tWRITE -> FLUSH     : %d\n" ANSI_COLOR_RESET, oracle.problem.write_flush);

    printf(ANSI_COLOR_YELLOW "Number of warnings         : %d\n" ANSI_COLOR_RESET, oracle.warning.total);
    printf(ANSI_COLOR_YELLOW "\tMOD ->  READ_ALLOC : %d\n" ANSI_COLOR_RESET, oracle.warning.mod_read_alloc);
    printf(ANSI_COLOR_YELLOW "\tMOD ->  WRITE      : %d\n\n" ANSI_COLOR_RESET, oracle.warning.mod_write);

    printf("Total Time      : %lf\n\n",elapsed_time);
    printf("Total MEM (peak): %zu Mbytes \n", getPeakRSS() );
    printf("Total MEM (cur) : %zu Mbytes \n", getCurrentRSS());

    return 0;
}

/* ==========================================================================
 *                             EXECUTION FLOW
 * ========================================================================== */

/**
 * @brief 
 *
 * @return 
 */
void *watson_pg_mem_init(void)
{
    return 0;
}

/**
 * @brief 
 *
 * @param cpu_i
 * @param regs_i
 *
 * @return 
 */
int watson_pg_exec_cpu(void *cpu_i, void *regs_i)
{
    return 0;
}


/**
 * @brief 
 *
 * @param cache
 * @param type
 * @param addr
 * @param e
 *
 * @return 
/ */
int watson_pg_exec_cache(void* cache, uint8_t type, uint32_t addr, void* e)
{
    hwe_cont* hwe     = event_content(e);

    uint32_t size = hwe->mem.body.mem32.width+1;
    hwe_cont* hwe_ref = event_content(event_ref(e,0)); // CACHE_REQ -> MEM_ACK

#ifdef DEBUG_ORDER
    printf("C[%ld] v [%d.%d] <- [%d.%d]\n",oracle.comp[hwe->common.id.devid]->cache_data.g_order,
             (uint32_t)hwe->common.id.devid,    (uint32_t)hwe->common.id.index,
             (uint32_t)hwe_ref->common.id.devid,(uint32_t)hwe_ref->common.id.index);

//  if(oracle.sys_order > oracle.comp[hwe->common.id.devid]->cache_data.g_order){
//      printf("Not well ordered\nG %ld\nC %ld\n", oracle.sys_order, oracle.comp[hwe->common.id.devid]->cache_data.g_order);
    }
#endif

    switch(type){
    case EVENT_ACK_READ:
    {
        reader_t *reader=NULL;
        int i;
        for (i = 0; i < size; i++){
            reader = watson_pending_read_ack((cache_t*)cache, addr+i);
            watson_event_to_reader(e, reader);
            PRINT_ADDR_DBG("A",addr+i,reader);
       }
    }
        break;
    case EVENT_ACK_MODIFY:
    {
        int i;
        modifier_t modifier;
        watson_event_to_writer(e,&modifier);
        modifier.pos_order = modifier.pre_order;
        for(i = 0; i < size; i++){ 
        //    reader = watson_pending_mod_ack((cache_t*)cache, addr+i);
        //    watson_event_to_reader(e, reader);
            PRINT_ADDR_DBG("M",addr+i, ((modifier_t*)&modifier));
            oracle.dfa_coherence(addr+i, cache, &modifier, EVENT_MODIFY_ACK);
        }
    }
        break;
    case EVENT_READ:
    {
        int i=0;
        reader_t reader;
        coherence_event_t read_event;

        watson_event_to_reader(e, &reader);
        reader.pos_order = reader.pre_order;

        read_event = hwe_ref->inst.body.str ? EVENT_ALLOC_REQ : EVENT_READ_REQ;

        for(i = 0; i < size; i++){
            if(read_event == EVENT_READ_REQ){
                if(((addr+i) < hwe->mem.body.mem32.cpureq_addr) ||
                   ((addr+i) > (hwe->mem.body.mem32.cpureq_addr + hwe->mem.body.mem32.cpureq_width))){
                    PRINT_ADDR_DBG("RL",addr+i,((reader_t*)&reader));
                    if(ADDR_DBG == addr+i)
                    printf("0x%08x %d\n", hwe->mem.body.mem32.cpureq_addr, hwe->mem.body.mem32.cpureq_width);
                    oracle.dfa_coherence(addr+i, cache, &reader, EVENT_ALLOC_REQ);
                }else{
                    PRINT_ADDR_DBG("RQ",addr+i,((reader_t*)&reader));
                    oracle.dfa_coherence(addr+i, cache, &reader, EVENT_READ_REQ);
                }
            }else{
               PRINT_ADDR_DBG("AL",addr+i,((reader_t*)&reader));
               oracle.dfa_coherence(addr+i, cache, &reader, EVENT_ALLOC_REQ);
            }
        }
    }
        break;
    case EVENT_WRITE:
    {
        int i;
        writer_t writer;
        watson_event_to_writer(e,&writer);
        writer.pos_order = writer.pre_order;
        for(i = 0; i < size; i++){
            PRINT_ADDR_DBG("W",addr+i,((writer_t*)&writer));
            oracle.dfa_coherence(addr+i, cache, &writer, EVENT_WRITE_REQ);
        }
    }    
        break;
    case EVENT_NOOP:
//      printf("addr = 0x%08x\n", addr);
//        ERROR("Event not valid");
        break;
        // WRITEBACK !!
        // MODIFY !!
    default:
        ERROR("Event not valid");
        break;
    }

//    if(meta_addr->mem_data.state == STATE_PROBLEM){
//       printf(" @ 0x%08x\n", addr ); 
//    }
    return 0;

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
int watson_pg_exec_mem(void* mem_i, void *addr_i, void* inst_i, uint8_t type, uint32_t addr, void* e)
{
    hwe_cont              *hwe = event_content(e);

    struct comp_data_t *cd_ref = comp_data(event_component(event_ref(e,0)));

//    hwe_cont              *hwe_ref = event_content(event_ref(e,0));

    cache_t             *cache;
    
    if (cd_ref->comp.cache == NULL){
        ERROR("No REF");
        return -1;
    }

#ifdef DEBUG_ORDER
    hwe_cont* hwe_ref          = event_content(event_ref(e,0)); // CACHE_REQ -> MEM_ACK
    printf("M[%d] v [%d.%d]\n",(uint32_t)hwe->common.id.index,
             hwe_ref->common.id.devid,(uint32_t)hwe_ref->common.id.index);
#endif

    if ( hwe->common.id.index > oracle.sys_order){
        oracle.sys_order = hwe->common.id.index;
    }else{
        ERROR("not well ordered");
    }

    cache = oracle.comp[cd_ref->comp.cache->id];
    watson_cache_g_order(cache, oracle.sys_order);

    switch(type){
    case EVENT_ACK_READ:
//        printf("MEM_READ_ACK\n");
        break;
    case EVENT_ACK_WRITE:
//        printf("MEM_WRITE_ACK\n");
        break;
    case EVENT_NOOP:
        break;
    default:
//        ERROR("Event not valid");
        break;
    }
    return 0;
}


/**
* @brief 
*
* @param dev
*
* @return 
*/
int watson_pg_exec_dev(void* dev)
{
    return 0;
}

/**
* @brief 
*
* @param addr
* @param comp
* @param comp_event
* @param event
*
* @return 
*/
#define LOCK_PC_SIZE 9
int32_t lock_pc[LOCK_PC_SIZE] = {
0x0001d4c0, // dna_memset
0x0001d528, // dna_memset   /  mcr 15, 0, r3, cr7, cr10, {1}
0x0002292c, // Lock release -> str
0x00022930, // Lock release -> flush
0x0001eab8, // Lock acquire -> ldrex
0x0001eac0, // Lock acquire -> strexeq
0x0001ead0, // cpu_compare_and_swap -> ldrex
0x0001ead8, // cpu_compare_and_swap -> strexeq
/*0x0001c5f8, // core_create -> error / strh    r5, [r4, #38]   ; 0x26
0x000031e4, // devfs_entry_unused_inspector -> error / ldr r1, [pc, #52]   ; 3220 <devfs_entry_unused_inspector+0x50>
0x0001af6c, // queue_add -> erroneous access
0x0001b0a4, // queue_add -> error  / mcr 15, 0, r3, cr7, cr10, {1}
0x0001b098, // queue_add -> error  / str r1, [r3]
0x0001a3d8, // queue_rem -> erroneous access  / mcr 15, 0, r3, cr7, cr10, {1}
0x0001a3cc, // queue_rem -> erroneour access  / str r3, [ip]
0x0001a364, // queue_rem -> erroneous access  / ldr lr, [ip]
0x00018498, // file_create -> error / mcr 15, 0, r2, cr7, cr10, {1}
0x00018490, // file_create -> error / str r1, [r2, #32]!
0x00018484, // file_create -> error / mcr 15, 0, r2, cr7, cr10, {1}
0x00018474, // file_create -> error / mcr 15, 0, r2, cr7, cr10, {1}
0x00018464, // file_create -> error / mcr 15, 0, r2, cr7, cr10, {1}
0x00018420, // file_create -> error / str r0, [r0, #56]   ; 0x38
0x0001842c, // file_create -> error / mcr 15, 0, r3, cr7, cr10, {1}
0x000192a8, // file_put  -> erroneous access
0x00019290, // file_put  -> erroneous access  / ldr r3, [r5, #32]
0x00018b08, // file_get  -> erroneous access  / ldrb    r6, [r5, #40]   ; 0x28
0x0001b2b8, // atomic_add -> erroneous access / ldr r5, [r6]
0x0001b2a4, // atomic_add -> error / mcr 15, 0, r2, cr7, cr14, {1}
0x000194a8, // vfs_write  -> erroneous access / mcr 15, 0, r3, cr7, cr10, {1}
0x0001941c, // vfs_write  -> erroneous access / ldr r1, [ip, #36]   ; 0x24
0x00019434, // vfs_write  -> erroneous access / ldr r1, [r1, #28]
0x0001dfd4, // scheduler_elect -> errouneous access / ldr r8, [pc, #252]  ; 1e0d8 <scheduler_elect+0x120>
0x00020594, // kernel_region_create -> error / mcr 15, 0, r3, cr7, cr10, {1}
0x00020568, // kernel_region_create -> error / ldr ip, [r5, #8]
0x00020600, // kernel_region_create -> error / mcr 15, 0, r3, cr7, cr10, {1}
0x00020428, // kernel_region_create -> error / mcr 15, 0, r3, cr7, cr10, {1} 
0x000203d8, // kernel_region_create -> error / mcr 15, 0, r3, cr7, cr10, {1} 
0x000203cc, // kernel_region_create -> error / str r8, [r6, #4]
0x000204a0, // kernel_region_create -> error / ldr r3, [ip, #8]
0x00020478, // kernel_region_create -> error / mcr 15, 0, r3, cr7, cr14, {1}
0x00020550, // kernel_region_create -> error / mcr 15, 0, r2, cr7, cr14, {1}
0x0001ff88, // kernel_region_create -> error / ldr r3, [r4]
0x00020380, // kernel_region_create -> error / ldr r3, [r4]
0x000200a8, // kernel_region_destroy -> erroneous access / mcr 15, 0, r2, cr7, cr10, {1}
0x0001ff50, // kernel_region_destroy -> error / mcr 15, 0, r3, cr7, cr10, {1}
0x0001fedc, // kernel_region_destroy -> error / mcr 15, 0, r3, cr7, cr10, {1}
0x0001ff0c, // kernel_region_destroy -> error / str lr, [ip, #8]
0x0002001c, // kernel_region_destroy -> error / mcr 15, 0, r0, cr7, cr10, {1}
0x00020268, // kernel_free -> error / mcr 15, 0, r3, cr7, cr10, {1}
0x00018048, // vnode_put -> error   / ldr r3, [r4, #24]
0x0001860c, // vnode_create -> error / str r3, [r8, #24]
0x00018184, // pthread_attr_setstackaddr -> error / sub sp, sp, #12
*/};


uint8_t watson_dfa_writeback(uint32_t addr, void* comp, void* comp_event,  coherence_event_t event)
{
 // TODO: remove comp and use comp_event->id instead
    sl_addr_t *meta_addr;

//    int i,j;

//    if( addr > STACK_START && addr < STACK_END){
//        return 0; 
//    }
    meta_addr =  mem_get_addr(addr);

/*    for( i = 0; i < LOCK_SIZE; i++){
        if(addr == lock[i]){
            for(j=0;j< LOCK_PC_SIZE; j++){
                if (((writer_t*)comp_event)->pc == lock_pc[j])
                    goto lock_test;
            }
            if (j == LOCK_PC_SIZE){
                PRINT_ADDR(addr);
                switch(event){
                case EVENT_MODIFY_ACK:
                    PRINT_MODIFIER((modifier_t*)comp_event);
                    break;
                case EVENT_WRITE_REQ:
                    PRINT_WRITER((writer_t*)comp_event);
                    break;
                case EVENT_READ_REQ:
                    PRINT_READER((reader_t*)comp_event);
                    break;
                case EVENT_READ_ACK:
                    PRINT_READER((reader_t*)comp_event);
                    ERROR("It does not happened\n");
                break;
                case EVENT_ALLOC_REQ:
                    PRINT_READER((reader_t*)comp_event);
                    printf("ALLOC\n");
                break;
                default:
                    ERROR("not recognied event");
                    printf("event %d\n", event);
                break;
                }
            }
        }
    }
lock_test:
*/
/*    if(addr == ADDR_DBG){
        printf("0x%08x state %d dev %d ev %d\n",addr, meta_addr->mem_data.state, ((writer_t*)comp_event)->id, event);
    }
*/
    switch(meta_addr->mem_data.state) {
    case STATE_INIT:
        switch(event){
        case EVENT_MODIFY_ACK:
        {
            writer_t* writer     = &meta_addr->mem_data.writer;
            modifier_t* modifier = &meta_addr->mem_data.modifier;
            reader_t* reader     = &meta_addr->mem_data.reader;
            uint32_t* bmp_readers= &meta_addr->mem_data.bitmap_readers;

            uint8_t   *state     = &meta_addr->mem_data.state;

            memset(writer,   0,          sizeof(writer_t));
            memcpy(modifier, comp_event, sizeof(modifier_t));
             memset(reader,   0,          sizeof(reader_t));
            *bmp_readers  = 0;
            *bmp_readers |= (1 << modifier->id);
            *state = STATE_MODIFY;
        }
            break;
        case EVENT_WRITE_REQ:
        case EVENT_READ_REQ:
            // Il faut guarder la derniere lecture
        case EVENT_WRITE_ACK:
        case EVENT_READ_ACK:
            // Il faut guarder la derniere lecture
            break;
        default:
            break;
        }
        break;
    case STATE_MODIFY:
        switch (event){
        case EVENT_READ_ACK:
        {
            reader_t   *reader   = &meta_addr->mem_data.reader;
            modifier_t *modifier = &meta_addr->mem_data.modifier;
            uint8_t    *state    = &meta_addr->mem_data.state;

            memcpy(reader,comp_event, sizeof(reader_t));

//            ASSERT_MOD_READ_ORDER(addr,modifier,reader);

            if(modifier->id != reader->id){
                ASSERT_MOD_READ_ORDER(addr,modifier,reader);
                PRINT_PROBLEM(addr,NULL,modifier,reader);
                oracle.problem.mod_read_ack++;
                ERROR("MODIFY->READ_ACK\n");
                *state = STATE_PROBLEM;
            }
        }
            break;
        case EVENT_READ_REQ:
        {
            modifier_t *modifier = &meta_addr->mem_data.modifier;
            reader_t   *reader   = &meta_addr->mem_data.reader;
            uint8_t    *state    = &meta_addr->mem_data.state;

            memcpy(reader,comp_event, sizeof(reader_t));

            ASSERT_MOD_READ_ORDER(addr,modifier,reader);
            PRINT_PROBLEM(addr,NULL,modifier,reader);
            oracle.problem.mod_read_req++;
            ERROR("MODIFY->READ_REQ\n");
            *state = STATE_PROBLEM;
        }
            break;
        case EVENT_ALLOC_REQ:
            if(meta_addr->mem_data.warn_addr == false){
                meta_addr->mem_data.warn_addr = true;
                oracle.warning.mod_read_alloc++;
                oracle.warning.total++;
                PRINT_WARNNING("MOD->ALLOC",addr,NULL,&meta_addr->mem_data.modifier,((reader_t*)comp_event));
            }
            break;
        case EVENT_MODIFY_ACK:
        {

            writer_t   *writer      = &meta_addr->mem_data.writer;
            reader_t   *reader      = &meta_addr->mem_data.reader;
            modifier_t *modifier    = &meta_addr->mem_data.modifier;
            uint32_t   *bmp_readers = &meta_addr->mem_data.bitmap_readers;
            uint8_t    *state       = &meta_addr->mem_data.state;
//            bool       *warn_addr   = &meta_addr->mem_data.warn_addr;

            ASSERT_READ_MOD_ORDER(addr,reader, comp_event); // There is no reader after
            ASSERT_MOD_MOD_ORDER(addr,modifier, comp_event);

            memset(writer,   0,          sizeof(writer_t));
            memset(reader,   0,          sizeof(reader_t));
            memcpy(modifier, comp_event, sizeof(modifier_t));

            *bmp_readers  = 0;
            *bmp_readers |= (1 << modifier->id);
//            *warn_addr   = false;

            *state = STATE_MODIFY;
        }
            break;
        case EVENT_WRITE_REQ:
        {
            writer_t   *writer      = &meta_addr->mem_data.writer;
            reader_t   *reader      = &meta_addr->mem_data.reader;
            modifier_t *modifier    = &meta_addr->mem_data.modifier;
            uint32_t   *bmp_readers = &meta_addr->mem_data.bitmap_readers;
            uint8_t    *state       = &meta_addr->mem_data.state;
            bool       *warn_addr   = &meta_addr->mem_data.warn_addr;

            if(modifier->id == ((writer_t*)comp_event)->id){
                ASSERT_MOD_WRIT_ORDER(addr,modifier, (writer_t*)comp_event);
                memcpy(writer,   comp_event, sizeof(writer_t));
                memset(reader,   0,          sizeof(reader_t));
                memset(modifier, 0,          sizeof(modifier_t));

                *bmp_readers  = 0;
                *bmp_readers |= (1 << writer->id);
                *state = STATE_WRITEBACK;
            }else{
                if(*warn_addr == false){
                    oracle.warning.mod_write++;
                    oracle.warning.total++;
                    *warn_addr   = true;
                    PRINT_WARNNING("MOD->STR", addr,((writer_t*)comp_event),modifier,NULL); 
                }
            }
        }
            break;
        default:
            ERROR("Not supported event");
            break;
        }
        break;
    case STATE_WRITEBACK:
        switch(event){
        case EVENT_READ_ACK:
        {
            writer_t *writer      = &meta_addr->mem_data.writer;
            writer_t *reader      = &meta_addr->mem_data.reader;
            uint32_t *bmp_readers = &meta_addr->mem_data.bitmap_readers; 
            uint8_t  *state       = &meta_addr->mem_data.state;

            memcpy(reader, comp_event, sizeof(reader_t));

            if( !(*bmp_readers & (1 << ((reader_t*)comp_event)->id)) ){
                ASSERT_WRIT_READ_ORDER(addr,writer, (reader_t*)comp_event);
                PRINT_PROBLEM(addr,writer,NULL,(reader_t*)comp_event); 
                oracle.problem.write_read_ack++;
                ERROR("WRITEBACK->READ_ACK\n");
                *state = STATE_PROBLEM;
            }
        }
            break;
        case EVENT_ALLOC_REQ:
        case EVENT_READ_REQ:
        {
            reader_t *reader      = &meta_addr->mem_data.reader;

            memcpy(reader, comp_event, sizeof(reader_t));

            meta_addr->mem_data.bitmap_readers |= (1 << ((reader_t*)comp_event)->id);
        }
            break;
        case EVENT_MODIFY_ACK:
        {
            writer_t   *writer    = &meta_addr->mem_data.writer;
            reader_t   *reader    = &meta_addr->mem_data.reader;
            modifier_t *modifier  = &meta_addr->mem_data.modifier;
            uint32_t *bmp_readers = &meta_addr->mem_data.bitmap_readers;
            uint8_t  *state       = &meta_addr->mem_data.state;
//            bool     *warn_addr   = &meta_addr->mem_data.warn_addr;

            ASSERT_WRIT_MOD_ORDER(addr,writer, comp_event);
            ASSERT_WRIT_READ_MOD_ORDER(addr,writer,reader, comp_event);

            memset(writer,            0, sizeof(writer_t));
//            memset(reader,            0,          sizeof(reader_t));
            memcpy(modifier, comp_event, sizeof(modifier_t));

            *bmp_readers  = 0;
            *bmp_readers |= (1 << modifier->id);

            *state = STATE_MODIFY;
        }
            break;
        case EVENT_WRITE_REQ:
        {
            writer_t*   writer   = &meta_addr->mem_data.writer;
            reader_t*   reader   = &meta_addr->mem_data.reader;
            modifier_t* modifier = &meta_addr->mem_data.modifier;
            uint32_t *bmp_readers = &meta_addr->mem_data.bitmap_readers;
            uint8_t  *state       = &meta_addr->mem_data.state;

            ASSERT_WRIT_WRIT_ORDER(addr,writer,comp_event);

            if(*bmp_readers  & (1 << ((writer_t*)comp_event)->id)){

                memcpy(writer, comp_event, sizeof(writer_t));
                memset(reader,   0, sizeof(reader_t));
                memset(modifier, 0, sizeof(modifier_t));
                *bmp_readers  = 0;
                *bmp_readers |= (1 << writer->id);

            }else{
                *state = STATE_PROBLEM;
                PRINT_PROBLEM(addr,writer,NULL,(reader_t*)comp_event); 
                oracle.problem.write_flush++;
                ERROR("WRITEBACK->FLUSH\n");
            }

        }
            break;
        default:
            ERROR("Not supported event");
            break;
        }
        break;
    case STATE_PROBLEM: 
        break;        
    default:
        ERROR("This state does not exist");
        break;
    }
    return 0;
}

/**
 * @brief 
 *
 * @param meta_addr
 * @param comp
 * @param comp_event
 * @param event
 *
 * @return -2 if this address had presented a problem, -1 if the global order of reader is early than writer 
 */
uint8_t watson_dfa_writethrough(uint32_t addr, void* comp, void* comp_event,  coherence_event_t event)
{
 // TODO: remove comp and use comp_event->id instead
    sl_addr_t *meta_addr =  mem_get_addr(addr);

    if(meta_addr->mem_data.bad_addr) return 0;

    switch(meta_addr->mem_data.state) {
    case STATE_INIT:
        switch(event){
        case EVENT_WRITE_REQ:
        {
            writer_t* writer = &meta_addr->mem_data.writer;
//            reader_t* reader = &meta_addr->mem_data.reader;

            meta_addr->mem_data.state = STATE_ANALYSE;
            memcpy(writer, comp_event, sizeof(writer_t));
//            memset(reader, 0,          sizeof(reader_t));
            meta_addr->mem_data.bitmap_readers  = 0;
            meta_addr->mem_data.bitmap_readers |= (1 << writer->id);
//            printf("BITMAP: 0x%08x@0x%08x WRITE_INIT\n", meta_addr->mem_data.bitmap_readers, addr);
        }
            break;
        case EVENT_READ_REQ:
//            ERROR("Ignored")
        case EVENT_WRITE_ACK:
        case EVENT_READ_ACK:
            break;
        default:
            break;
        }
        break;
    case STATE_ANALYSE:
        switch(event){
        case EVENT_WRITE_REQ:
        {
            writer_t* writer = &meta_addr->mem_data.writer;
//          reader_t* reader = &meta_addr->mem_data.reader;

            if(writer->pos_order > ((writer_t*)comp_event)->pos_order) { 
                PRINT_WRITER(writer);
                PRINT_WRITER(comp_event);
                ERROR("Writers in wrong order");
                exit(1);
            }
            memcpy(writer, comp_event, sizeof(writer_t));
//            memset(reader, 0, sizeof(reader_t));
            meta_addr->mem_data.bitmap_readers  = 0;
            meta_addr->mem_data.bitmap_readers |= (1 << writer->id);
        }
            break;
        case EVENT_READ_REQ:
        {
            writer_t* writer = &meta_addr->mem_data.writer;
            reader_t *reader = (reader_t*)comp_event;
            if(writer->pre_order > reader->pos_order) {
                ERROR("wrong order");
//                exit(1);
            }
            if(writer->pos_order > reader->pre_order) { 
                ERROR("erroneours reader");
 //               exit(1);
            }
            meta_addr->mem_data.bitmap_readers |= (1 <<  reader->id);
        }
            break;
        case EVENT_WRITE_ACK:
            ERROR("Not yet");
            break;
        case EVENT_READ_ACK:
        {
            writer_t *writer      = &meta_addr->mem_data.writer;
            reader_t *reader_test = (reader_t*)comp_event;
            uint32_t  bmp_readers = meta_addr->mem_data.bitmap_readers; 

            if( !(bmp_readers & (1 << reader_test->id)) ){
                if(writer->pos_order > reader_test->pre_order) { 
                    ((cache_t*)comp)->cache_data.stats.false_positive++;
                    ERROR("Not possible to define");
                    meta_addr->mem_data.bad_addr = true;
                    meta_addr->mem_data.state = STATE_PROBLEM;
                    PRINT_ADDR(addr);
                    PRINT_WRITER(writer);
                    PRINT_READER(reader_test);
                    PRINT_SRC_WRITER(writer);
                    PRINT_SRC_READER(reader_test);
//                    exit(1);
                    return 0;
                }

//                 reader_t* reader;
//                 reader = &meta_addr->mem_data.reader;

//                 memcpy(reader, comp_event, sizeof(reader_t));

                 meta_addr->mem_data.state = STATE_PROBLEM;

                 meta_addr->mem_data.bad_addr = true;

                 PRINT_ADDR(addr);
                 PRINT_WRITER(writer);
                 PRINT_READER((reader_t*)comp_event);
                 PRINT_SRC_WRITER(writer);
                 PRINT_SRC_READER((reader_t*)comp_event);
                 
            }
        }
            break;
        default:
            ERROR("Not yet");
            break;
        }
        break;
    case STATE_PROBLEM:
        break;
    default:
        ERROR("Not yet");
        break;
    }
    return 0;
}

/**
 * @brief 
 *
 * @param cache
 * @param order
 */
void watson_cache_g_order(cache_t* cache, uint64_t order)
{
    uint32_t i;
    cache_data_t *cache_data =  &cache->cache_data;
    uint32_t n_pend  =  cache_data->n_pend_read;

    cache_data->g_order = order;

    // Update pending global orders and analyse them
/*    if(oracle.write_policy == WRITEBACK){
        uint32_t n_pend_mod  =  cache_data->n_pend_mod;
        

        for(i = 0; i < n_pend_mod; i++){ // MODIFIERS
            struct pend_mod_t *pend =  &cache_data->pend_mod[i]; 
            reader_t *modifier = &pend->modifier;
            uint32_t addr    = pend->addr;
            modifier->pos_order = order;

            oracle.dfa_coherence(addr, cache, modifier, EVENT_MODIFY_ACK);
        }
        cache_data->n_pend_mod = 0;
    }*/

    for(i = 0; i < n_pend; i++){
        struct pend_read_t *pend =  &cache_data->pend_read[i]; 
        reader_t *reader = &pend->reader;
        uint32_t addr    = pend->addr;
        reader->pos_order = order;

        oracle.dfa_coherence(addr, cache, reader, EVENT_READ_ACK);
    }
    cache_data->n_pend_read = 0;
       
}
    
reader_t *watson_pending_read_ack(cache_t* cache, uint32_t addr)
{
    cache_data_t *cache_data = &cache->cache_data;
    uint32_t      n_pend     = cache_data->n_pend_read;
    struct pend_read_t *pend;

    if(n_pend > MAX_PENDING) {
        ERROR("Read pending overflow\n");
        exit(1);
    }

    pend       = &cache_data->pend_read[n_pend];
    pend->addr = addr;
    n_pend++;
    cache_data->n_pend_read = n_pend;

    // Stats 
    if (cache_data->n_pend_read > cache_data->stats.max_n_pend_read){
        cache_data->stats.max_n_pend_read = cache_data->n_pend_read; 
    }
    return &pend->reader;
}

reader_t *watson_pending_mod_ack(cache_t* cache, uint32_t addr)
{

    cache_data_t *cache_data = &cache->cache_data;
    uint32_t      n_pend     = cache_data->n_pend_mod;
    struct pend_mod_t *pend;

    if(n_pend > MAX_PENDING) {
        ERROR("Read pending overflow\n");
        exit(1);
    }

    pend       = &cache_data->pend_mod[n_pend];
    pend->addr = addr;
    n_pend++;
    cache_data->n_pend_mod = n_pend;

    // Stats 
    if (cache_data->n_pend_mod > cache_data->stats.max_n_pend_mod){
        cache_data->stats.max_n_pend_mod = cache_data->n_pend_mod; 
    }
    return  &pend->modifier;
}

void watson_event_to_reader(event_t *e, reader_t* reader)
{
   
    hwe_cont* hwe     = event_content(e);
    hwe_cont* hwe_ref = event_content(event_ref(e,0)); // CACHE_REQ -> MEM_ACK

    if (reader == NULL){
        ERROR("NULL reader");
        exit(1);
    }

    reader->id         = hwe->common.id.devid;
    reader->pc         = hwe_ref->inst.body.pc;
    reader->pre_order  = oracle.comp[hwe->common.id.devid]->cache_data.g_order;
    reader->pos_order  = 0;
#ifndef SMALL_MEMORY
    reader->comp_order = hwe->common.id.index;
    reader->cpu_id     = hwe_ref->common.id.devid;
    reader->cpu_index  = hwe_ref->common.id.index;
    reader->timestamp  = hwe_ref->common.dates[0];
#endif
}

void watson_event_to_writer(event_t *e, writer_t* writer)
{
    if (writer == NULL){
        ERROR("NULL writer");
        exit(1);
    }

    hwe_cont* hwe     = event_content(e);
    hwe_cont* hwe_ref = event_content(event_ref(e,0)); // CACHE_REQ -> MEM_ACK

    writer->id         = hwe->common.id.devid;
    writer->pc         = hwe_ref->inst.body.pc;
    writer->pre_order  = oracle.comp[hwe->common.id.devid]->cache_data.g_order;
    writer->pos_order  = 0;
#ifndef SMALL_MEMORY    
    writer->comp_order = hwe->common.id.index;
    writer->cpu_id     = hwe_ref->common.id.devid;
    writer->cpu_index  = hwe_ref->common.id.index;
    writer->timestamp  = hwe_ref->common.dates[0];
#endif
}

void PRINT_READER(reader_t *reader)
{
    if(reader ==NULL) return;
#ifdef SMALL_MEMORY
    printf("R%d\tpc @ 0x%08x\tORDER[%ld.%ld]\n",
            reader->id,
            reader->pc,
            reader->pre_order,
            reader->pos_order); 
#else
    printf("R%d\tpc @ 0x%08x\n\t[%d.%ld]->[%d.%ld]\tORDER[%ld.%ld]\ttime: %ld\n",
            reader->id,
            reader->pc,
            reader->cpu_id,
            reader->cpu_index,
            reader->id,
            reader->comp_order,
            reader->pre_order,
            reader->pos_order,
            reader->timestamp);
#endif
}
void PRINT_SRC_READER(reader_t *reader)
{
    char *fname = "";
    uint32_t lineno = 0;

    insn_desc_t instr = {0,0};

    if(reader == NULL) return;

    instr.pc = reader->pc;

    dh_addr2line(dbg_linux, &instr, &lineno, &fname);
    printf("READER @ 0x%08x\n", instr.pc);
    printf("%s:%d\n", fname, lineno);
#if 0
    {
    char str[256];
    int i;

    sprintf(str,"arm-sls-dnaos-addr2line -e %s 0x%08x\n", is.elf_filename, reader->pc /* | 0xc0000000*/);
    i = system(str);
    if(i == -1)
        printf("Something went wrong with System()! %s\n", strerror(errno));
    printf("\n");
    }
#endif
}


void PRINT_WRITER(writer_t *writer)
{
    if(writer == NULL) return;

#ifdef SMALL_MEMORY
    printf("W%d\tpc @ 0x%08x\tORDER[%ld.%ld]\n",
            writer->id,
            writer->pc,
            writer->pre_order,
            writer->pos_order); 
#else
    printf("W%d\tpc @0x%08x\n\t[%d.%ld]->[%d.%ld]\tORDER[%ld.%ld]\ttime: %ld\n",
            writer->id,
            writer->pc,
            writer->cpu_id,
            writer->cpu_index,
            writer->id,
            writer->comp_order,
            writer->pre_order,
            writer->pos_order,
            writer->timestamp);
#endif
}



void PRINT_SRC_WRITER(writer_t *writer)
{
    char *fname = "";
    uint32_t lineno = 0;

    insn_desc_t instr = {0,0};

    if(writer == NULL) return;

    instr.pc = writer->pc;

    dh_addr2line(dbg_linux, &instr, &lineno, &fname);
    printf("WRITER @ 0x%08x\n", instr.pc);
    printf("%s:%d\n", fname, lineno);
#if 0
    {
    char str[256];
    int i;

    sprintf(str,"arm-sls-dnaos-addr2line -e %s 0x%08x", is.elf_filename, writer->pc /*| 0xc0000000*/);
    i = system(str);
    if(i == -1)
        printf("Something went wrong with System()! %s\n", strerror(errno));
    printf("\n");
    }
#endif
}

void PRINT_MODIFIER(modifier_t *writer)
{
    if(writer == NULL) return;
#ifdef SMALL_MEMORY
    printf("M%d\tpc @ 0x%08x\tORDER[%ld.%ld]\n",
            writer->id,
            writer->pc,
            writer->pre_order,
            writer->pos_order);
#else
    printf("M%d\tpc @0x%08x\n\t[%d.%ld]->[%d.%ld]\tORDER[%ld.%ld]\ttime: %ld\n",
            writer->id,
            writer->pc,
            writer->cpu_id,
            writer->cpu_index,
            writer->id,
            writer->comp_order,
            writer->pre_order,
            writer->pos_order,
            writer->timestamp);
#endif

}

void PRINT_SRC_MODIFIER(writer_t *writer)
{
    char *fname = "";
    uint32_t lineno = 0;

    insn_desc_t instr = {0,0};

    if(writer == NULL) return;

    instr.pc = writer->pc;

    dh_addr2line(dbg_linux, &instr, &lineno, &fname);
    printf("MODIFIER @ 0x%08x\n", instr.pc);
    printf("%s:%d\n", fname, lineno);
#if 0
    {
    char str[256];
    int i;

    sprintf(str,"arm-sls-dnaos-addr2line -e %s 0x%08x", is.elf_filename, writer->pc /*| 0xc0000000*/);
  i = system(str);
    if(i == -1)
        printf("Something went wrong with System()! %s\n", strerror(errno));
    printf("\n");
    }
#endif
}


void PRINT_ADDR(uint32_t addr)
{
    char str[256];
    int i;

    sprintf(str, "arm-sls-dnaos-nm -e %s | grep %08x", is.elf_filename, addr /*| 0xc0000000*/);
    printf(ANSI_COLOR_RED "ADDRESS: 0x%08x\n" ANSI_COLOR_RESET, addr);
    i = system(str);
    if(i == -1)
        printf("Something went wrong with System()! %s\n", strerror(errno));

}

// TEMPORARY FUNCTIONS 
inst_node_t *oracle_get_last_exec(void)
{
    return NULL;
}

void gdb_verify(comp_type_t type_i, gdb_param_t *param)
{

}

SL_PLUGIN_INIT(watson_pg_init)
SL_PLUGIN_EXIT(watson_pg_exit)
SL_PLUGIN_COMP(watson_pg_comp)
SL_PLUGIN_MEM_INIT(watson_pg_mem_init)
SL_PLUGIN_EXEC_CPU(watson_pg_exec_cpu)
SL_PLUGIN_EXEC_CACHE(watson_pg_exec_cache)
SL_PLUGIN_EXEC_MEM(watson_pg_exec_mem)
SL_PLUGIN_EXEC_DEV(watson_pg_exec_dev)


