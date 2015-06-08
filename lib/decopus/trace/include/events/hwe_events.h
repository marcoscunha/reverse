

#include "hwe_common.h"
#include "hwe_info.h"
#include "hwe_inst.h"
#include "hwe_mem.h"

#ifndef __HWE_EVENTS_H__
#define __HWE_EVENTS_H__

/*
 * event container
 */
typedef union hwe_cont hwe_cont;
union hwe_cont {
   hwe_head_cont   common;
   hwe_info_cont   info;
   hwe_inst_cont   inst;
   hwe_cpu_cont    cpu;
   hwe_excep_cont  excep;
   hwe_mem_cont    mem;
   hwe_ack_cont    ack;
};

#endif

