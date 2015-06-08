#ifndef __DNP_RDMA_DRIVER_PRIVATE_H__
#define __DNP_RDMA_DRIVER_PRIVATE_H__

#include <DnpRdmaDriver/Driver.h>
#include <DnaTools/DnaTools.h>

#include <Private/Dnp.h>

#if 0
static inline uint32_t dnp_readl(dnp_device_t *dev, uint32_t off){
	 uint32_t res;
	 return cpu_read(UINT32, dev->mmio0+off, res);
}

static inline void dnp_writel(dnp_device_t *dev, uint32_t off, uint32_t val){
	 cpu_write(UINT32, dev->mmio0+off, val);
}
#endif


typedef struct dnp_rdma_file dnp_rdma_file_t;

enum channel_status {
  CHANNEL_NOT_INITIALIZED,
  CHANNEL_READY,
  CHANNEL_RIP,              /* read in progress */
};

struct dnp_rdma_file {
  rdma_channel_t *channel;
 
  uint32_t        status;
  uint32_t        lbuf_size; /* in words */
  uint32_t        lbuf_pos;  /* in words */
  uint32_t       *lbuffer; 
};



/*
 * Driver operations
 */
extern status_t      dnp_rdma_init_hardware (void);
extern status_t      dnp_rdma_init_driver (void);
extern void          dnp_rdma_uninit_driver (void);
extern const char  **dnp_rdma_publish_devices (void);
extern device_cmd_t *dnp_rdma_find_device (const char * name);

extern uint32_t  *dnp_channels_virt_to_dev;

/*
 * Devfs operations
 */
extern int32_t dnp_rdma_isr (void * data);
extern status_t dnp_rdma_control (void * handler, int32_t function,
								  va_list arguments, int32_t * p_ret);
extern status_t dnp_rdma_open (char * name, int32_t mode, void ** data);
extern status_t dnp_rdma_close (void * data);
extern status_t dnp_rdma_free (void * data);
extern status_t dnp_rdma_read (void * handler, void * destination,
							   int64_t offset, int32_t * p_count);
extern status_t dnp_rdma_write (void * handler, void * source,
								int64_t offset, int32_t * p_count);



#endif /* __DNP_RDMA_DRIVER_PRIVATE_H__ */
