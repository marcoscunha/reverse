#ifndef _CORE_CPU_H_
#define _CORE_CPU_H_

#include <cfg.h>
#include <hwetrace.h>
#include <events/hwe_events.h>
#include <events/hwe_device.h>
#include <hwe_handle_def.h>

// FOR ARM PROCESSOR * XXX: Must be the same in QEMU trace generation

#define USER_REGS 0b00010000
#define FIQ_MODE  0b00100000

#define REGID_CPSR 0xFF
#define REGID_SPSR 0xFE

#define BANK_SPSR 0x40

#define CPSR_MODE 0x1f

#define N_GPREG 16

#define ARM_REGS 37 // TODO: To be verified when backward is being implemented

typedef struct{
    uint32_t data; // Modify its name to "value"
#ifdef REG_METADATA
    REG_METADATA;
#endif
}reg_t;

typedef enum arm_cpu_mode {
  ARM_CPU_MODE_USR = 0x10,
  ARM_CPU_MODE_FIQ = 0x11,
  ARM_CPU_MODE_IRQ = 0x12,
  ARM_CPU_MODE_SVC = 0x13,
  ARM_CPU_MODE_ABT = 0x17,
  ARM_CPU_MODE_UND = 0x1b,
  ARM_CPU_MODE_SYS = 0x1f
} arm_cpu_mode_t; 

#define REGID_USR 0 << 4
#define REGID_SYS 0 << 4 

#define REGID_FIQ 1 << 4
#define REGID_IRQ 2 << 4
#define REGID_SVC 3 << 4 
#define REGID_ABT 4 << 4
#define REGID_UND 5 << 4 

typedef struct {
    uint8_t        comp_type;       /*Must be the first field in all components */
    hwe_id_dev_t   id;
    arm_cpu_mode_t mode;
    reg_t regs[16];

    reg_t cpsr;
    reg_t spsr;

    /* Banked Register */
    reg_t banked_spsr[6];
    reg_t banked_r13[6];
    reg_t banked_r14[6];

    /* These Hold r8-r12 */
    reg_t usr_regs[5];
    reg_t fiq_regs[5];

    //XXX: Does not support coprocessors yet!
#ifdef CPU_METADATA
    CPU_METADATA;
#endif
}cpu_t;

typedef struct{
    uint32_t nreg;
    uint32_t id[18];
    uint32_t data[18];
}exec_reg_t;

/*
typedef struct{
    uint32_t regid;
    uint32_t data;
} reg_exec_t;

typedef struct{
    uint32_t    nreg;
    reg_exec_t *regs[REG_EXEC_MAX];
}cpu_exec_t;
*/

/* HANDLE EVENTS */
void   cpu_init(event_t *e);

/* SIMULATION EVENTS */
cpu_t* cpu_create(event_t *e);

void cpu_exec(cpu_t *cpu, exec_reg_t *regs);

uint32_t cpu_bank(uint32_t mode);

void switch_mode(cpu_t *cpu, uint32_t mode);

//void cpu_reverse_exec(cpu_t* cpu);


#endif // _CORE_CPU_H_
