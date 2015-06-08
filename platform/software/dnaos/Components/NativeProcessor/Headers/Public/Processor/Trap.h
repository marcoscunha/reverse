#ifndef PROCESSOR_TRAP_H
#define PROCESSOR_TRAP_H

#include <stdint.h>

typedef enum exception_id
{
  cpu_trap_data_abort,
  cpu_trap_prefetch_abort,
  cpu_trap_undefined
}
exception_id_t;

typedef uint32_t interrupt_id_t;
typedef uint32_t interrupt_status_t;
typedef int32_t (* exception_handler_t) (void);
typedef int32_t (* interrupt_handler_t) (void * data);

extern interrupt_handler_t * cpu_handler_table[8];

extern void __dnaos_hal_trap_attach_esr (exception_id_t id, exception_handler_t isr);
extern void __dnaos_hal_trap_attach_isr (interrupt_id_t id, uint32_t mode, interrupt_handler_t isr);

extern interrupt_status_t __dnaos_hal_trap_mask_and_backup (void);
extern void __dnaos_hal_trap_restore (interrupt_status_t backup);

extern void __dnaos_hal_trap_enable (interrupt_id_t id);
extern void __dnaos_hal_trap_disable (interrupt_id_t id);

#define cpu_trap_count()	\
	8

/* extern void cpu_trap_attach_esr (int32_t cpuid, exception_id_t id,
    exception_handler_t isr); */
/* extern void cpu_trap_attach_isr (int32_t cpuid, interrupt_id_t id,
    uint32_t mode, interrupt_handler_t isr); */
#define cpu_trap_attach_esr(cpuid, id, isr)	\
  __dnaos_hal_trap_attach_esr(id,isr)
#define cpu_trap_attach_isr(cpuid, id, mode, isr)	\
  __dnaos_hal_trap_attach_isr(id,mode,isr)

#define cpu_trap_mask_and_backup()	\
  __dnaos_hal_trap_mask_and_backup()

#define cpu_trap_restore(backup)	\
  __dnaos_hal_trap_restore(backup)

#define cpu_trap_enable(id)	\
  __dnaos_hal_trap_enable(id)

#define cpu_trap_disable(id)	\
  __dnaos_hal_trap_disable(id)

#endif

