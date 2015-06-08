#ifndef SYSTEM_D940_EMAC_H
#define SYSTEM_D940_EMAC_H
#include <stdint.h>

/* /!\ Don't change it /!\ */
#define D940_ETH_ID 5
#define D940_ETH_RX_BUFFER_SIZE 128
#define D940_ETH_MAX_BUFFER 1024

typedef volatile union 
{
  struct 
  {
    uint32_t lb:1;          /* Loopback */
    uint32_t llb:1;         /* Loopback local */
    uint32_t re:1;          /* Receive enable */
    uint32_t te:1;          /* Transmit enable */
    uint32_t mpe:1;         /* Management port enable */
    uint32_t clrstat:1;     /* Cleat statistics registers */
    uint32_t incstat:1;     /* Increment statistics registers */
    uint32_t westat:1;      /* Write enable for statistics registers */
    uint32_t bp:1;          /* Back pressure */
    uint32_t tstart:1;      /* Start transmission */
    uint32_t thalt:1;       /* Transmit halt */
    uint32_t __padding:21;
  } bits;
  uint32_t raw;
} d940_eth_ncr_t;

typedef volatile union
{
  struct 
  {
    enum spd_enum
    {
      speed_10mb = 0,
      speed_100mb = 1
    } spd:1;                /* Speed */
    uint32_t fd:1;          /* Full duplex */
    uint32_t __undef0:1;
    uint32_t jframe:1;      /* Jumbo Frames */
    uint32_t caf:1;         /* Copy All Frames */
    uint32_t nbc:1;         /* No Broadcast */
    uint32_t mti:1;         /* Multicast Hash Enable */
    uint32_t uni:1;         /* Unicast Hash Enable */
    uint32_t big:1;         /* Receive 1536 bytes frames */
    uint32_t __undef1:1;
    enum clk_enum
    {
      clk_div8 = 0,
      clk_div16 = 1,
      clk_div32 = 2,
      clk_div64 = 3
    } clk:2;                /* MDC clock divider */
    uint32_t rty:1;         /* Retry test */
    uint32_t pae:1;         /* Pause Enable */
    uint32_t rbof:2;        /* Receive Buffer Offset */
    uint32_t rlce:1;        /* Receive Length field Checking Enable */
    uint32_t drfcs:1;       /* Discard Receive FCS */
    uint32_t efrhd:1;       /* */
    uint32_t irxfcs:1;      /* Ignore RX FCS */
    uint32_t __padding:12;
  } bits;
  uint32_t raw;
} d940_eth_ncfgr_t;

typedef volatile union
{
  struct
  {
    uint32_t __undef0:1;
    uint32_t mdio:1;        /* */
    uint32_t idle:1;        /* */
    uint32_t __padding:29;
  } bits;
  uint32_t raw;
} d940_eth_nsr_t;

typedef volatile union
{
  struct
  {
    uint32_t ubr:1;         /* Used Bit Read */
    uint32_t col:1;         /* Collision Occurred */
    uint32_t rle:1;         /* Retry Limit exceeded */
    uint32_t tgo:1;         /* Transmit Go */
    uint32_t bex:1;         /* Buffers exhausted mid frame */
    uint32_t comp:1;        /* Transmit Complete */
    uint32_t und:1;         /* Transmit Underrun */
    uint32_t __padding:25;
  } bits;
  uint32_t raw;
} d940_eth_tsr_t;

typedef volatile union
{
  struct
  {
    uint32_t addr;          /* buffer queue pointer address */
  } bits;
  uint32_t raw;
} d940_eth_qp_t;

typedef volatile union
{
  struct
  {
    uint32_t bna:1;         /* Buffer Not Available */
    uint32_t rec:1;         /* Frame Received */
    uint32_t ovr:1;         /* Receive Overrun */
    uint32_t __padding:29;
  } bits;
  uint32_t raw;
} d940_eth_rsr_t;

typedef volatile union
{
  struct
  {
    uint32_t mfd:1;         /* Management Frame sent */
    uint32_t rcomp:1;       /* Receive Complete */
    uint32_t rxubr:1;       /* Receive Used Bit Read */
    uint32_t txubr:1;       /* Transmit Used But Read */
    uint32_t tund:1;        /* Ethernet Transmit Buffer Underrun */
    uint32_t rle:1;         /* Retry Limit Exceeded */
    uint32_t txerr:1;       /* */
    uint32_t tcomp:1;       /* Transmit Complete */
    uint32_t __undef0:1;      
    uint32_t __undef1:1;      
    uint32_t rovr:1;        /* Receive Overrun */
    uint32_t hresp:1;       /* Hresp not OK */
    uint32_t pfr:1;         /* Pause Frame Received */
    uint32_t ptz:1;         /* Pause Time zero */
    uint32_t __padding:18;
  } bits;
  uint32_t raw;
} d940_eth_int_t;

typedef volatile struct 
{
  uint32_t addr_l;
  uint32_t addr_h;
} d940_eth_addr_t;

typedef volatile union
{
  struct
  {
    uint32_t data:16;         /* DATA */
    uint32_t code:2;          /* CODE */
    uint32_t rega:5;          /* Register Address */
    uint32_t phya:5;          /* Phy Address */
    uint32_t rw:2;            /* Read/Write */
    uint32_t sof:2;           /* Start of frame */
  } bits;
  uint32_t raw;
} d940_eth_man_t;


typedef volatile union
{
  struct
  {
    uint32_t rmii:1;          /* RMII mode */
    uint32_t clken:1;         /* Enable transceiver input clock */
    uint32_t __padding:30;
  } bits;
  uint32_t raw;
} d940_eth_usrio_t;

typedef volatile struct
{
  d940_eth_ncr_t            ncr;          /* Network Control Register */
  d940_eth_ncfgr_t          ncfgr;        /* Network Configuration Register */
  volatile uint32_t         __reserved0;
  volatile uint32_t         __reservec1;
  d940_eth_nsr_t            nsr;          /* Network Status Register */
  d940_eth_tsr_t            tsr;          /* Transmit Status Register */
  d940_eth_qp_t             rbqp;         /* Receive Buffer Queue Pointer Register */
  d940_eth_qp_t             tbqp;         /* Transmit Buffer Queue Pointer Register */
  d940_eth_rsr_t            rsr;          /* Receive Status Register */
  d940_eth_int_t            isr;          /* Interrupt Status Register */
  d940_eth_int_t            ier;          /* Interrupt Enable Register */
  d940_eth_int_t            idr;          /* Interrupt Disable Register */
  d940_eth_int_t            imr;          /* Interrupt Mask Register */
  d940_eth_man_t            man;          /* Phy Maintenance Register */
  volatile uint32_t         ptr;          /* Pause Timer Register */
  volatile uint32_t         pfr;          /* Pause Frames Received Register */
  volatile uint32_t         fto;          /* Frame Transmitted Ok Register */
  volatile uint32_t         scf;          /* Single Collision Frames Register */
  volatile uint32_t         mcf;          /* Multiple Collision Frames Register */
  volatile uint32_t         fro;          /* Frame Received  Ok Register */
  volatile uint32_t         fcse;         /* Frame Check Sequence Errors Register */
  volatile uint32_t         ale;          /* Alignement Errors Register */
  volatile uint32_t         dtf;          /* Deferred Transmission Frames Register */
  volatile uint32_t         lcol;         /* Late Collisions Register */
  volatile uint32_t         ecol;         /* Excessive Collisions Register */
  volatile uint32_t         tund;         /* Transmit Underrun Errors Register */
  volatile uint32_t         cse;          /* Carrier Sense Errors Register */
  volatile uint32_t         rre;          /* Receive Ressource Errors Register */
  volatile uint32_t         rov;          /* Receive Overrun Errors Register */
  volatile uint32_t         rse;          /* Receive Symbol Errors Register */
  volatile uint32_t         ele;          /* Excessive Length Errors Register */
  volatile uint32_t         rja;          /* Receive Jabbers Register */
  volatile uint32_t         usf;          /* Undersize Frames Register */
  volatile uint32_t         ste;          /* SQE Test Errors Register */
  volatile uint32_t         rle;          /* Received Length Field Mismatch Register */
  volatile uint32_t         __undef0;
  volatile uint64_t         hr;           /* Hash Register Register */
  d940_eth_addr_t           sa1;          /* Specific Address 1 Register */
  d940_eth_addr_t           sa2;          /* Specific Address 2 Register */
  d940_eth_addr_t           sa3;          /* Specific Address 3 Register */
  d940_eth_addr_t           sa4;          /* Specific Address 4 Register */
  volatile uint32_t         tid;          /* Type ID Checking Register */
  volatile uint32_t         __undef1;
  d940_eth_usrio_t          usrio;        /* User Input/Output Register */
} * d940_eth_t;

struct rbde
{
  uint32_t          owner:1;      /* Ownership */
  uint32_t          wrap:1;       /* Wrap - marks last descriptor */
  uint32_t          addr:30;      /* Adresse of beginning of buffer */

  uint32_t          len:12;       /* Length of the frame */
  uint32_t          offset:2;     /* Receive buffer offset */
  uint32_t          start:1;      /* Start of frame */
  uint32_t          end:1;        /* End of frame */
  uint32_t          cfi:1;        /* Concatenation format indicator */
  uint32_t          vlan_prio:3;  /* VLAN priority */
  uint32_t          prio_tag:1;   /* Priority tag detected */
  uint32_t          vlan_tag:1;   /* VLAN tag detected */
  uint32_t          type_id:1;    /* Type ID match */
  uint32_t          sa4:1;        /* Specific address register 4 match */
  uint32_t          sa3:1;        /* Specific address register 3 match */
  uint32_t          sa2:1;        /* Specific address register 2 match */
  uint32_t          sa1:1;        /* Specific address register 1 match */
  uint32_t          __reserved:1; 
  uint32_t          ea:1;         /* External address match */
  uint32_t          uni_hash:1;   /* Unicast hash match */
  uint32_t          multi_hash:1; /* Multicast hash match */
  uint32_t          boardcast:1;  /* Global all ones broadcast address detected */
};

struct tbde
{
  uint32_t          addr;         /* Byte Address  of buffer */

  uint32_t          len:11;       /* Length of buffer */
  uint32_t          __reserved:4; 
  uint32_t          last:1;       /* Last buffer */
  uint32_t          no_crc:1;     /* No CRC */
  uint32_t          __reserved2:10;
  uint32_t          exhausted:1;  /* Buffer exhausted in mid frame */
  uint32_t          underrun:1;   /* Transmit underrun */
  uint32_t          retry_limit:1;/* Rety limi exceeded */
  uint32_t          wrap:1;       /* Marks last descriptor */
  uint32_t          used:1;       /* Used buffer marker */

};

#endif
