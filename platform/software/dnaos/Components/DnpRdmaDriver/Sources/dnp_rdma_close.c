#include <DnaTools/DnaTools.h>

#include <Private/defines.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>


status_t
dnp_rdma_close(void * data ATTRIBUTE_UNUSED) {

	 DMSG("close\n");
	 return DNA_OK;
}

