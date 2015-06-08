#ifndef __DNP_RDMA_DRIVER_H__
#define __DNP_RDMA_DRIVER_H__

#include <DnaTools/DnaTools.h>

enum dnp_rdma_requests
{
  DNP_CONNECT = DNA_CONTROL_CODES_END
};

typedef struct rdma_channel rdma_channel_t;
struct rdma_channel {
	 uint32_t  id;

	 uint32_t  use_float;
	 uint32_t  tgt_rank;
	 uint32_t  tgt_cpu_id; // id of the processor of the tile
	 uint32_t  eager_rdv_threshold;
	 uint32_t  non_blocking;

  uint32_t dummy1;
  uint32_t dummy2;
  uint32_t dummy3;
  uint32_t dummy4;

};

typedef struct dnp_settings dnp_settings_t;
struct dnp_settings{
  /*  IRQ Number */
  uint32_t irq_no;

  /* Event queue buffer physical address */
  uint32_t  *evq_buffer;
  
  /* Eventqueue buffer size */
  uint32_t  evq_size;
  
  /* Streaming Buffer physical address */
  uint32_t  nstreaming_buffer;
  
  /* Streaming Buffer size */
  uint32_t  streaming_size;
};

typedef struct rdma_common rdma_common_t;
struct rdma_common{
	 uint32_t local_rank;
	 uint32_t  local_cpu_id; // id of the processor of the tile
};


typedef struct topo_settings topo_settings_t;

struct topo_settings{

	 uint32_t  mesh_ndims;
	 uint32_t  mesh_dims[5];
};

extern driver_t        dnp_rdma_module;

extern dnp_settings_t  DNP_SETTINGS;

extern rdma_common_t   DNP_COMMON;
extern rdma_channel_t *DNP_CHANNELS;
extern uint32_t        DNP_CHANNELS_NDEV;
extern uint32_t        DNP_CHANNELS_NVIRT;

extern topo_settings_t DNP_TOPOLOGY_INFO;

/*
 * To be changed
 */
extern uint32_t  DNP_BASE_ADDR;

#endif /* __DNP_RDMA_DRIVER_H__ */
