#include <DnaTools/DnaTools.h>
#include <DnpRdmaDriver/Driver.h>
#include <Private/Dnp.h>
#include <Private/defines.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma:isr"
#include <Private/debug.h>


int32_t
dnp_rdma_isr(void * data ATTRIBUTE_UNUSED){

  uint32_t status;
  dnp_reg_read(INTRSTS1_IDX, status);
  DMSG("rdma_isr: interrupts: :0x%08x\n", status);

  if( status & IRQ_RDMA_M) {
    DMSG("RDMA interrupt\n");
    dnp_reg_write(INTRSTS1_IDX, IRQ_RDMA_M); // ack 
    dnp_event_poll();
  }
  if( status & IRQ_RDMA_EXC_M) {
    uint32_t rdmaexc = 0;
    dnp_reg_read(RDMAEXC_IDX, rdmaexc);
    dnp_reg_write(RDMAEXC_IDX, rdmaexc);

    EMSG("RDMA exception interrupt: 0x%08x\n", rdmaexc);
    dnp_reg_write(INTRSTS1_IDX, IRQ_RDMA_EXC_M);
  }
  if( status & IRQ_ENGINE_EXC_M) {
    uint32_t engexc = 0;
    dnp_reg_read(ENGEXC_IDX, engexc);
    dnp_reg_write(ENGEXC_IDX, engexc);

    EMSG("Engine exception interrupt: 0x%08x\n", engexc);
    dnp_reg_write(INTRSTS1_IDX, IRQ_ENGINE_EXC_M);
  }

  return DNA_INVOKE_SCHEDULER;
}

