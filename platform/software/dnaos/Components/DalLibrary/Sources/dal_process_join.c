#include <stdint.h>
#include <Private/DalLibrary.h> 
#include <Core/Semaphore.h> 

volatile uint32_t __dal_nb_alive_processes = 0;
int32_t __dal_sem;

void dal_process_join(void) {
  while(__dal_nb_alive_processes > 0) {
    semaphore_acquire(__dal_sem, 1, 0, 0);
    __dal_nb_alive_processes--;
    dna_printf("DAL: SEMAPHORE_ACQUIRED, REMAINING:%d\n", __dal_nb_alive_processes);
  }
}

