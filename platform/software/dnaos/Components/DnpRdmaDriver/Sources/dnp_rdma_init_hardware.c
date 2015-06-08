#include <DnaTools/DnaTools.h>
#include <MemoryManager/MemoryManager.h>

#include <Private/Driver.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>

status_t
dnp_rdma_init_hardware(void) {

	 DMSG("init_hardware\n");

	 rdma_engine_init();

	 return DNA_OK;
}


