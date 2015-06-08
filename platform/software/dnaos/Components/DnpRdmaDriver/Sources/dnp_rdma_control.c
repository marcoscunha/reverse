#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>
#include <Private/defines.h>

#include <Private/Driver.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>

status_t
dnp_rdma_control(void * handler, int32_t function, va_list arguments,
		 int32_t * p_ret){
  
  dnp_rdma_file_t *file = (dnp_rdma_file_t *)handler;
  int32_t virt_channel  = va_arg(arguments, int32_t);
  int32_t channel_idx = -1;

  DMSG("control()\n");

  switch (function)
    {
    case DNP_CONNECT:
      
      channel_idx = dnp_channels_virt_to_dev[virt_channel];
      if( (virt_channel >= DNP_CHANNELS_NVIRT) ||
	  (channel_idx == -1)                  ){
	EMSG("Error channel id unknown (%d/%d)\n", virt_channel, channel_idx);
	return DNA_ERROR;
      }else{
	DMSG("Connecting to channel (%d/%d)\n", virt_channel, channel_idx);
      }

      file->channel = &DNP_CHANNELS[channel_idx];

      rdma_engine_open(file->channel->id);

      file->lbuffer = kernel_malloc(file->channel->eager_rdv_threshold*sizeof(uint32_t), false);
      file->lbuf_pos  = 0;
      file->lbuf_size = 0;

      file->status = CHANNEL_READY;

      *p_ret = 0;
      break;
      
    default:
      return DNA_ERROR;
    }

  return DNA_OK;
}
