#include <DalLibrary/DalLibrary.h>
#include <Core/Core.h>

#include <fcntl.h>
#include <unistd.h>

dal_status_t dal_read(void *port, void *data, int32_t n, DALProcess *p) {
  //#ifdef DAL_USE_DNP
  int res; 
  DalChannel *channel = (DalChannel *) port; 

  if(channel->is_local == 0) {  
    res = read(channel->fd, data, n);
    if(res == 0) {
      // Nicolas non blocking implementation
      return DAL_ERROR;
    }
    if(res == -1) {
      //dna_printf("DAL: kpn read error\n");
      return DAL_ERROR;
    }
    // TODO: CD: is it possible to have a partial read (0 < res < n) ?

    return DAL_OK;
    //#else
    //  DalChannel *channel = (DalChannel *) port;
  } else {
    if(channel->non_blocking && channel->start == channel->end) {
      return DAL_ERROR;
    }

    int index = 0;
    while(index < n) {
      if(channel->start != channel->end) {
        int next = (channel->start + 1) % channel->size;
        ((unsigned char *)data)[index++] = channel->buffer[channel->start];
        channel->start = next;
      }
      else {
        thread_yield();
      }
    }

    return DAL_OK;
    //#endif
  }
}

