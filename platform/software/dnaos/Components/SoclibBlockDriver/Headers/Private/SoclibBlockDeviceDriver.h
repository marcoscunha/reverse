#ifndef DNAOS_SOCLIB_BLOCK_DEVICE_DRIVER_PRIVATE_H
#define DNAOS_SOCLIB_BLOCK_DEVICE_DRIVER_PRIVATE_H

#include <Core/Core.h>
#include <DnaTools/DnaTools.h>
#include "block_device.h"

typedef struct _block_device_register_map
{
  uint32_t  BLOCK_DEVICE_BUFFER ;
  uint32_t  BLOCK_DEVICE_LBA ;
  uint32_t  BLOCK_DEVICE_COUNT ;
  uint32_t  BLOCK_DEVICE_OP ;
  uint32_t  BLOCK_DEVICE_STATUS ;
  uint32_t  BLOCK_DEVICE_IRQ_ENABLE ;
  uint32_t  BLOCK_DEVICE_SIZE ;
  uint32_t  BLOCK_DEVICE_BLOCK_SIZE ;
} * block_device_register_map_t ;

typedef struct _block_device_control
{
  int32_t semaphore_id ;
  int32_t isr_semaphore_id ;
  bool    should_enable_irq ;
  int32_t irq ;
  int32_t block_count ;
  int32_t block_size ;
  block_device_register_map_t port ;
}
block_device_control_t ;

typedef struct _block_device
{
  int32_t should_enable_irq ;
  int32_t irq ;
  int32_t base_address ;
}
block_device_t ;

typedef enum _block_device_access {
  READ,
  WRITE
} 
block_device_access_t ;

extern int32_t SOCLIB_BLOCK_DEVICES_NDEV ;
extern block_device_t SOCLIB_BLOCK_DEVICES[] ;

extern block_device_control_t * block_device_controls ;
extern const char * block_devices[] ;
extern device_cmd_t block_device_commands ;

extern status_t block_device_init_hardware (void) ;
extern status_t block_device_init_driver (void) ;
extern void block_device_uninit_driver (void) ;
extern const char ** block_device_publish_devices (void) ;
extern device_cmd_t * block_device_find_device (const char * name) ;

extern int32_t block_device_isr (void * data) ;

extern status_t block_device_open (char * name, int32_t mode, void ** data) ;
extern status_t block_device_close (void * data) ;
extern status_t block_device_free (void * data) ;
extern status_t block_device_read (void * handler, void * destination, 
    int64_t offset, int32_t * p_count) ;
extern status_t block_device_write (void * handler, void * source, 
    int64_t offset, int32_t * p_count) ;
extern status_t block_device_control (void * handler, int32_t function,
    va_list arguments, int32_t * p_ret) ;

extern status_t access_device_blocks(block_device_control_t * block_device, 
    void * access, int64_t block_offset, int32_t block_count,
    block_device_access_t read_or_write) ;

#endif
