/*
 * Copyright (C) 2007 TIMA Laboratory                                    
 *                                                                       
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or     
 * (at your option) any later version.                                   
 *                                                                       
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef SYSTEM_D940_H
#define SYSTEM_D940_H

#include <Core/Semaphore.h>
#include <DnaTools/DnaTools.h>
#include <Public/Ethernet/mii.h>
#include "pmc.h"
#include "emac.h"
#include "pio.h"

#define D940_ETH_PHY_STATUS_RETRY 10
#define D940_ETH_READ_TIMEOUT 500000000

/* Have to be a multiple of 4 */
#define D940_ETH_TX_BUFFER_SIZE   128 

/* Limit (-1) of packet in transmission */ 
#define TX_PACKET_LIMIT           4

/* MAX 1024 */
#define D940_ETH_TX_BUFFER_COUNT  512 

/* MAX 1024 */
#define D940_ETH_RX_BUFFER_COUNT  512 

#define NEXT_TX_BUFFER(x) ((x + 1) % D940_ETH_TX_BUFFER_COUNT)
#define NEXT_RX_BUFFER(x) ((x + 1) % D940_ETH_RX_BUFFER_COUNT)

typedef struct d940_eth_data
{
  unsigned char transmit_buffers[D940_ETH_TX_BUFFER_SIZE
                                * D940_ETH_TX_BUFFER_COUNT];
  unsigned char receive_buffers[D940_ETH_RX_BUFFER_SIZE 
                                * D940_ETH_RX_BUFFER_COUNT];
  struct tbde   transmit_descs[D940_ETH_TX_BUFFER_COUNT];
  struct rbde   receive_descs[D940_ETH_RX_BUFFER_COUNT];
 
  /* Access */
  spinlock_t  lock;
  int32_t     ref;

  /* Device */
  d940_eth_t  dev;
  int32_t     it;

  /* PHY */
  int32_t   mio_sem;
  int32_t   mio_comp_sem;
  int32_t   phy_id;
  uint32_t  phy_status;

  /* Store index in list */
  int32_t tx_tail;
  int32_t rx_tail;

  /* Rx & Tx Semaphore */ 
  int32_t tx_sem;
  int32_t tx_write;

  int32_t rx_sem;
  bool    rx_read;
  
  /* Stats */
  int32_t tx_count;
  int32_t rx_count;

} d940_eth_data_t;

extern d940_eth_t         PLATFORM_ETH_BASE; 

extern d940_eth_data_t *  d940_ethernet_handlers[];
extern const char *       d940_ethernet_devices[];

extern status_t         d940_ethernet_init_hardware (void);
extern status_t         d940_ethernet_init_driver (void);
extern void             d940_ethernet_uninit_driver (void);
extern const char **    d940_ethernet_publish_devices (void);
extern device_cmd_t *   d940_ethernet_find_device (const char * name);

extern status_t   d940_ethernet_close (void * handler);
extern status_t   d940_ethernet_control (void * handler, int32_t function,
                    void * arguments, int32_t * p_ret);
extern status_t   d940_ethernet_open (char * name, int32_t mode, void ** data);
extern status_t   d940_ethernet_read (void * handler, void * destination,
                    int64_t offset, int32_t * p_count);
extern status_t   d940_ethernet_write (void * handler, void * source,
                    int64_t offset, int32_t * p_count);


extern int32_t  d940_ethernet_isr (void * data);
extern bool     d940_ethernet_phy_probe(d940_eth_data_t *pdata);
extern void     d940_ethernet_phy_manage(d940_eth_data_t *pdata);

extern void     d940_ethernet_mdio_disable(d940_eth_data_t *pdata);
extern void     d940_ethernet_mdio_enable(d940_eth_data_t *pdata);
extern uint16_t d940_ethernet_mdio_read(d940_eth_data_t *pdata, int reg_id);
extern void     d940_ethernet_mdio_write(d940_eth_data_t *pdata,
                  int reg_id, uint16_t val);
#endif
