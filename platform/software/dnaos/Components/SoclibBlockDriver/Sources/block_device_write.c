
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>


status_t block_device_write (void * handler, void * source, 
        int64_t offset, int32_t * p_count)
{
  int32_t block_start, block_end, nb_blocks ;
  void * bulk_buffer, * bulk_buffer_base_address, * source_buffer ;
  int32_t local_offset, local_count, remaining_bytes, quot ;

  block_device_control_t * block_device = (block_device_control_t *) handler ;

  dna_log(VERBOSE_LEVEL, "Writing %d bytes with offset %d from source 0x%x",
      *p_count, offset, source) ;
  watch (status_t)
  {
    semaphore_acquire (block_device -> semaphore_id, 1, 0, -1) ;

    block_start = offset / block_device -> block_size ;
    block_end = (offset + *p_count) / block_device -> block_size ;
    nb_blocks = block_end - block_start + 1 ;

    bulk_buffer = kernel_malloc (nb_blocks *
        block_device -> block_size, false) ;
    bulk_buffer_base_address = bulk_buffer ;
    ensure (bulk_buffer != NULL, DNA_OUT_OF_MEM) ;

    ensure (
        access_device_blocks (block_device, 
                           bulk_buffer, 
                           offset / block_device -> block_size, 
                           nb_blocks,
                           READ) 
        == DNA_OK, DNA_ERROR) ;

    cpu_cache_invalidate (CPU_CACHE_DATA, bulk_buffer, 
        block_device -> block_size * nb_blocks) ;

    local_offset = offset % block_device -> block_size ;
    bulk_buffer += local_offset ;
    remaining_bytes = *p_count ;
    source_buffer = source ;

    local_count = remaining_bytes > block_device -> block_size - local_offset ? block_device -> block_size - local_offset : remaining_bytes ;
    
    if (local_count > 0) {
      dna_memcpy (bulk_buffer, source_buffer, local_count) ;
      bulk_buffer += local_count ;
      source_buffer += local_count ;
      remaining_bytes -= local_count ;
    }

    quot = remaining_bytes / block_device -> block_size ;
    while (quot > 0)
    {
      dna_memcpy (bulk_buffer, source_buffer, block_device -> block_size) ;
      bulk_buffer += block_device -> block_size ;
      source_buffer += block_device -> block_size ;
      remaining_bytes -= block_device -> block_size ;
      quot-- ;
    }

    if (remaining_bytes > 0)
    {
      dna_memcpy (bulk_buffer, source_buffer, remaining_bytes) ;
    }

    cpu_cache_invalidate (CPU_CACHE_DATA, bulk_buffer_base_address, 
        block_device -> block_size * nb_blocks) ;

    ensure (access_device_blocks (block_device, 
                                  bulk_buffer_base_address,
                                  block_start,
                                  nb_blocks,
                                  WRITE)
        == DNA_OK, DNA_ERROR) ;

    dna_log(VERBOSE_LEVEL, "Wrote %d block(s) from block %d, using 0x%x as a source address", 
        nb_blocks, block_start, bulk_buffer_base_address) ;
    
    kernel_free (bulk_buffer_base_address) ;
    semaphore_release (block_device -> semaphore_id, 1, DNA_NO_RESCHEDULE) ;
    return DNA_OK ;
  }

  return DNA_NOT_IMPLEMENTED ;
}
