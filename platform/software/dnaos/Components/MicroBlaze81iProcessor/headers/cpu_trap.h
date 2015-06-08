#ifndef CPU_TRAP_H
#define CPU_TRAP_H

#include <stdint.h>

typedef enum exception_id {
	CPU_TRAP_DATA_ABORT,
	CPU_TRAP_PREFETCH_ABORT,
	CPU_TRAP_UNDEFINED
} exception_id_t;

typedef uint32_t interrupt_id_t;
typedef uint32_t interrupt_status_t;
typedef int32_t (* exception_handler_t) (void);
typedef int32_t (* interrupt_handler_t) (int32_t itn);

#define CPU_N_IT 1

extern interrupt_handler_t handler_table[CPU_N_IT];

extern void CPU_TRAP_ATTACH_ESR (exception_id_t id, exception_handler_t isr);
extern void CPU_TRAP_ATTACH_ISR (interrupt_id_t id, uint32_t mode, interrupt_handler_t isr);

extern interrupt_status_t CPU_TRAP_MASK_AND_BACKUP (void);
extern void CPU_TRAP_RESTORE (interrupt_status_t backup);

extern void CPU_TRAP_ENABLE (interrupt_id_t id);
extern void CPU_TRAP_DISABLE (interrupt_id_t id);

#endif

