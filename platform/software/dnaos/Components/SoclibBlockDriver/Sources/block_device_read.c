
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

status_t block_device_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count) 
{
  void * read_buffer, * read_buffer_base_address ;
  uint32_t nb_blocks ;
  uint32_t block_size ;
  uint32_t lba ;
  uint32_t local_offset, local_p_count, remaining_bytes, quot;

  block_device_control_t * block_device = (block_device_control_t *) handler ;

  dna_log(VERBOSE_LEVEL, "Reading %d bytes with offset %d at destination 0x%x", 
      *p_count, (uint32_t) offset, destination) ;
  watch (status_t)
  {

    ensure (offset + *p_count <= 
        block_device -> block_count * block_device -> block_size, 
        DNA_BAD_ARGUMENT) ;
    block_size = block_device -> block_size ;

    semaphore_acquire (block_device -> semaphore_id, 1, 0, -1) ;

    nb_blocks = ((*p_count / block_size + 1) +
        ((offset + *p_count) / block_size > 0 &&
         (offset + *p_count) % block_size > 0 ?
         1 :
         0)) ;
    lba = offset / block_size ;

    read_buffer = kernel_malloc (nb_blocks * block_size, true) ;
    read_buffer_base_address = read_buffer ;
    ensure (read_buffer != NULL, DNA_OUT_OF_MEM) ;
    
    ensure (access_device_blocks(block_device, 
                                 read_buffer,
                                 lba,
                                 nb_blocks,
                                 READ) 
        == DNA_OK,
        DNA_ERROR) ;

    cpu_cache_invalidate (CPU_CACHE_DATA, read_buffer,
        block_size * nb_blocks) ;

    // Trimming the blocks to fit the p_count size and the block offset
    DCACHE_INVAL(p_count,sizeof(uint32_t*));
    remaining_bytes = *p_count ;

    // Remainder from first block
    //
    local_offset = offset % block_size ;
    local_p_count = remaining_bytes > (block_size - local_offset) ?
      remaining_bytes - (block_size - local_offset) :
      remaining_bytes ;
    read_buffer += local_offset ;
    dna_memcpy (destination, read_buffer, local_p_count) ;

    remaining_bytes -= local_p_count ;
    destination += local_p_count ;
    read_buffer += local_p_count ;

    // Block transfers
    quot = remaining_bytes / block_size ;
    while (quot > 0)
    {
      dna_memcpy (destination, read_buffer, block_size) ;
      remaining_bytes -= block_size ;
      destination += block_size ;
      read_buffer += block_size ;
      quot-- ;
    }

    // Remainder from last block
    if (remaining_bytes > 0)
    {
      dna_memcpy (destination, read_buffer, remaining_bytes) ;
    }

    kernel_free (read_buffer_base_address) ;
    semaphore_release (block_device -> semaphore_id, 1, DNA_NO_RESCHEDULE) ;
    return DNA_OK ;
  }
}
