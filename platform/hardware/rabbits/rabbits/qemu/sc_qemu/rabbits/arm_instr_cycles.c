#include <string.h>
#include "rabbits/arm_instr_cycles.h"

static int arm_default_instr_cycles[] =
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

int *get_arm_cpu_instr_cycles (const char *cpumodel)
{
    return arm_default_instr_cycles;
}
