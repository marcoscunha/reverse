#ifndef _ARM_INSTR_CYCLES_H_
#define	_ARM_INSTR_CYCLES_H_

int *get_arm_cpu_instr_cycles (const char *cpumodel);

enum arm_instr_cycle_t
{
    ARM_NORMAL_INSTRUCTION,
    ARM_JUMP,
    ARM_LOAD,
    ARM_STORE,
    ARM_SIGNED_MUL,
    ARM_REGISTER_SHIFT,
    ARM_MUL8,
    ARM_MUL16,
    ARM_MUL24,
    ARM_MUL32,
    ARM_MUL64,
    ARM_MLS,
    ARM_MLA,
    ARM_MLAA,	/* ? */
    ARM_SWP,
    ARM_MULTI_TRANSFER_PER_REGISTER,
    ARM_MULTI_TRANSFER_LOAD_OP,
    ARM_MULTI_TRANSFER_STORE_OP,
    ARM_COCPU,	/* ? */
    ARM_COCPU_MRC,
    ARM_INSTR_CYCLES_NO_ITEMS
};

#endif /* _ARM_INSTR_CYCLES_H_ */
