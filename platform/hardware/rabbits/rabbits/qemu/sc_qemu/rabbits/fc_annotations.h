#ifndef _FC_ANNOTATIONS_
#define _FC_ANNOTATIONS_

#include "qemu-common.h"
#include "cpu.h"

void tb_start (TranslationBlock *tb);

#ifdef RABBITS_IMPLEMENT_CACHES
void icache_access (target_ulong addr);

void icache_access_n (target_ulong addr, int32_t n);

void icache_tr_tb_access(target_ulong addr, target_ulong insn, target_ulong type);
void icache_tr_inst_access(target_ulong addr, target_ulong insn, target_ulong type);
void icache_tr_jmp_access(target_ulong  addr, target_ulong dest, target_ulong type);

void restore_single_step (void);

void helper_mark_exclusive (void);
void helper_clrex (void);
int32_t helper_test_exclusive (void);

#endif

#ifdef RABBITS_GDB_ENABLED
void gdb_verify (target_ulong addr);
#endif

#ifdef RABBITS_LOG_INFO
void log_pc (target_ulong addr);
#endif

#ifdef RABBITS_PERF
void exec_perf(bool cmd);
#endif


#endif
