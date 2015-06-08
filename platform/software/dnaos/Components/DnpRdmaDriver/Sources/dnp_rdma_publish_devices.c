#include <Private/Driver.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>


const char *dnp_rdma_devices[] =
  {
    "dnp/rdma",
    NULL
  };

const char **
dnp_rdma_publish_devices(void) {

	 DMSG("publish_devices\n");

	 return dnp_rdma_devices;
}



