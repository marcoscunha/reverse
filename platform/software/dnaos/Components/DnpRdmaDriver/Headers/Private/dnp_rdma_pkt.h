#ifndef __DNP_RDMA_PKT_H__
#define __DNP_RDMA_PKT_H__


/*
 * Packet type definition
 */
typedef enum rdma_pkt_types rdma_pkt_types_t;

enum rdma_pkt_types {
    RDMA_PKT_NONE, 
    RDMA_PKT_RDV_INIT, 
    RDMA_PKT_RDV_END,
    RDMA_PKT_EAGER
};

/*
 * Internal classification of an Eager packet 
 * (multi-packet handling)
 */
typedef enum rdma_get_ret rdma_retval_t;

enum rdma_get_ret {
   RDMA_RETVAL_NONE = 0, 
   RDMA_RETVAL_DONE = 1, 
   RDMA_RETVAL_MULTI = 2,
};

/*
 * Definition of EAGER and rendez-vous headers
 */
typedef struct _pkt_eager pkt_eager_t;
typedef struct _pkt_rdv_init pkt_rdv_init_t;
typedef struct _pkt_rdv_end pkt_rdv_end_t;
typedef struct _rdma_pkt rdma_pkt_t;

struct _pkt_eager {
   uint32_t buf_nwords;
}; 

struct _pkt_rdv_init {
   uint32_t buf_address;
   uint32_t buf_nwords;
};

struct _pkt_rdv_end {
   uint32_t ack;
};

struct _rdma_pkt {
   rdma_pkt_types_t pkt_type;
   uint32_t channel_id;
   uint32_t use_float;
   union {
      pkt_eager_t     eager_pkt;
      pkt_rdv_init_t  init_pkt;
      pkt_rdv_end_t   end_pkt;
   } pkt;
};

#endif /* __DNP_RDMA_PKT_H__ */

/*
 * Vim standard variables
 * vim:set ts=3 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
