#ifndef _OP_ANNOTATIONS_
#define _OP_ANNOTATIONS_

#include "rabbits/fc_annotations.h"
#include "rabbits/qemu_systemc.h"
#include "rabbits/cfg.h"

#ifdef RABBITS_TRACE_EVENT 
#include "rabbits/trace_power.h"
#endif

static inline void gen_op_fc_call_0p (tcg_target_long fc)
{
    TCGv_ptr            f;

    f = tcg_const_ptr (fc);
    tcg_gen_callN (&tcg_ctx, f, 0, 0/*sizemask*/, 
        dh_retvar_void, 0/*no_args*/, NULL/*args*/);

    tcg_temp_free_ptr (f);
}

static inline void gen_op_fc_call_1p (tcg_target_long fc, tcg_target_long param)
{
    TCGv_ptr            f;
    TCGArg              args[1];
    int                 sizemask = 0;

    f = tcg_const_ptr (fc);
    args[0] = tcg_const_ptr (param);
    dh_sizemask(ptr, 1);
    tcg_gen_callN (&tcg_ctx, f, 0, sizemask, dh_retvar_void, 1, args);

    tcg_temp_free_ptr (args[0]);
    tcg_temp_free_ptr (f);
}

static inline void gen_op_fc_call_2p (tcg_target_long fc, tcg_target_long param1, int32_t param2)
{
    TCGv_ptr            f;
    TCGArg              args[2];
    int                 sizemask = 0;

    f = tcg_const_ptr (fc);
    args[0] = tcg_const_ptr (param1);
    dh_sizemask(ptr, 1);
    args[1] = tcg_const_i32 (param2);
    dh_sizemask(ptr, 2);
    tcg_gen_callN (&tcg_ctx, f, 0, sizemask, dh_retvar_void, 2, args);

    tcg_temp_free_ptr (args[0]);
    tcg_temp_free_ptr (args[1]);
    tcg_temp_free_ptr (f);
}

/**
 * @function
 *
 * @param fc ...
 * @param param1 ...
 * @param param2 ...
 * @param param3 ...
 *
 * @return void
 *
 */
static inline void gen_op_fc_call_3p (tcg_target_long fc,
									  tcg_target_ulong param1,
		                              tcg_target_ulong param2,
		                              tcg_target_ulong param3)
{

    TCGv_ptr f;
    TCGArg   args[3];
    int      sizemask = 0;

    f = tcg_const_ptr (fc);
    args[0] = tcg_const_ptr (param1);
    dh_sizemask(ptr, 1);
    args[1] = tcg_const_i32 (param2);
    dh_sizemask(ptr, 2);
    args[2] = tcg_const_i32 (param3);
	dh_sizemask(ptr, 3);

    tcg_gen_callN (&tcg_ctx, f, 0, sizemask, dh_retvar_void, 3, args);

    tcg_temp_free_ptr (args[0]);
    tcg_temp_free_ptr (args[1]);
    tcg_temp_free_ptr (args[2]);
    tcg_temp_free_ptr (f);
}

static inline void gen_op_tb_start (void *tb)
{
    gen_op_fc_call_1p ((tcg_target_long) tb_start, (tcg_target_long) tb);
}

#ifdef RABBITS_IMPLEMENT_CACHES
static inline void gen_op_icache_access_n (target_ulong addr, int32_t n)
{
	gen_op_fc_call_2p ((tcg_target_long) icache_access_n, addr, n);
}

static inline void gen_op_icache_access (target_ulong addr)
{
    gen_op_fc_call_1p ((tcg_target_long) icache_access, addr);
}
#endif

#ifdef RABBITS_GDB_ENABLED
static inline void gen_op_gdb_verify (target_ulong addr)
{
    gen_op_fc_call_1p ((tcg_target_long) gdb_verify, addr);
}
static inline void gen_op_restore_single_step (void)
{
    gen_op_fc_call_0p ((tcg_target_long) restore_single_step);
}
#endif

#ifdef RABBITS_LOG_INFO
static inline void gen_op_log_pc (target_ulong addr)
{
    gen_op_fc_call_1p ((tcg_target_long) log_pc, addr);
}
#endif

static inline void gen_op_inc_crt_nr_cycles (CPUState *env, int idx)
{
    if(!enable_trace){
        int nc;
        if (idx >= 10000)
            nc = idx - 10000;
        else
            nc = env->rabbits.instr_cycles[idx];
        if (!nc)
            return;

#if 1
        tcg_gen_op2ii (INDEX_op_rabbits_direct_add, 
                (tcg_target_long) &g_crt_no_cycles_instr, nc);

#else
        TCGv_ptr    addr = tcg_const_ptr ((tcg_target_long) &g_crt_no_cycles_instr);
        TCGv        t1 = tcg_temp_new_i32();
        TCGv t2 = tcg_const_i32 (nc);

        tcg_gen_ld_i32 (t1, addr, 0);
        tcg_gen_add_i32 (t1, t1, t2);
        tcg_gen_st_i32 (t1, addr, 0);

        tcg_temp_free_i32 (t1);
        tcg_temp_free_i32 (t2);
        tcg_temp_free_ptr (addr);
#endif
    }
}

#ifdef RABBITS_TRACE_EVENT
/**
 * @brief
 *
 * @param addr ...
 * @param type ...
 *
 * @return
 *
 */
static inline void gen_op_icache_tr_tb_access(target_ulong addr,
		                                      target_ulong insn,
		                                      target_ulong type)
{
    if(enable_trace){
        gen_op_fc_call_3p((tcg_target_long) icache_tr_tb_access, addr, insn, type);
    }else{
        gen_op_fc_call_1p ((tcg_target_long) icache_access, addr);
    }
}

/**
 * @brief
 *
 * @param addr ...
 * @param insn ...
 * @param type ...
 *
 * @return
 */
static inline void gen_op_icache_tr_inst_access(target_ulong addr,
												target_ulong insn,
                                                target_ulong type)
{
    if(enable_trace){
        gen_op_fc_call_3p((tcg_target_long)icache_tr_inst_access, addr, insn, type);
    }
}

/**
 * @brief
 *
 * @param addr ...
 * @param type ...
 *
 * @return void
 */
static inline void gen_op_icache_tr_jmp_access (target_ulong addr,
                                                target_ulong dest,
                                                target_ulong type)
{
    if(enable_trace){
        gen_op_fc_call_3p((tcg_target_long)icache_tr_jmp_access, addr, dest, type);
    }else{
        gen_op_fc_call_2p ((tcg_target_long) icache_access_n, addr, 2);
    }
}

/**
 * @brief
 * @param insn
 */
static inline void gen_op_tr_commit(target_ulong pc, target_ulong is_jmp)
{
    if(enable_trace){
        gen_op_fc_call_2p ((tcg_target_long) tr_commit, pc, is_jmp);
    }
}
#endif


#endif
