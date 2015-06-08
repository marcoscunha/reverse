#include <DalLibrary/DalLibrary.h>
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

dal_status_t dal_write(void *port, void *data, int32_t n, DALProcess *p) {
  //#ifdef DAL_USE_DNP
  int res;
  DalChannel *channel = (DalChannel *) port;

  if (channel->is_local == 0) {
    res = write(channel->fd, data, n);
    if(res == -1) {
      dna_printf("DAL: kpn write error\n");
      return DAL_ERROR;
    }
    // TODO: CD: is it possible to have a partial write (0 < res < n) ?

    return DAL_OK;
    //#else
    //  DalChannel *channel = (DalChannel *) port;
  }
  else {
    int i;
    for(i=0; i<n; i++) {
      int next = (channel->end + 1) % channel->size;
      if(next == channel->start) {
        if(channel->size < 32768) {
          //dna_printf("DAL: channel %s is full (%d bytes) : reallocating %d bytes\n", channel->name, channel->size, channel->size * 2);
          unsigned int newsize = channel->size * 2;
          unsigned char *newbuf = kernel_malloc(newsize, false);
          if(!newbuf) {
            dna_printf("memory allocation error ...\n");
            while(1);
          }
          if(channel->start == 0) {
            memcpy(newbuf, channel->buffer, channel->size);
          }
          else {
            unsigned int p0_size = channel->start;
            unsigned int p1_size = channel->size - p0_size;
            memcpy(newbuf, channel->buffer + channel->start, p1_size);
            memcpy(newbuf + p1_size, channel->buffer, channel->start);
          }
          free(channel->buffer);
          channel->buffer = newbuf;
          channel->start = 0;
          channel->end = channel->size - 1;
          channel->size = newsize;
          next = (channel->end + 1) % channel->size;
        }
        else {
          //dna_printf("DAL: channel %s is full, waiting for consumers to read ...\n", channel->name);
          while(next == channel->start) {
            thread_yield();
          }
        }
      }
      channel->buffer[channel->end] = ((unsigned char *)data)[i];
      channel->end = next;
    }

    //thread_yield();

    return DAL_OK;
    //#endif
  }
}

