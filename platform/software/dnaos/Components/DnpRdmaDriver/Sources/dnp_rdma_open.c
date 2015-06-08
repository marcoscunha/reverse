#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

#include <Private/Driver.h>
#include <Private/defines.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>

status_t
dnp_rdma_open(char * name,
	      int32_t mode ATTRIBUTE_UNUSED,
	      void ** data) {

	 char *base            = "dnp/rdma";
	 int base_len          = dna_strlen(base);
	 dnp_rdma_file_t *file = NULL;

	 DMSG("open (%s)\n", name);

	 if(dna_strncmp(name, base, base_len) != 0){
		  DMSG("name error\n");
		  return DNA_ERROR;
	 }

	 file = kernel_malloc(sizeof(dnp_rdma_file_t), false);

	 file->lbuffer   = NULL;
	 file->status    = CHANNEL_NOT_INITIALIZED;
	 file->lbuf_pos  = 0;
	 file->lbuf_size = 0;

	 *data = (void *)file;

	 return DNA_OK;
}



