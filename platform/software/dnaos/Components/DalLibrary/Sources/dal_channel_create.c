#include <Core/Core.h>
#include <DalLibrary/DalLibrary.h>
#include <MemoryManager/MemoryManager.h>

//#ifdef DAL_USE_DNP
#include <DnpRdmaDriver/Driver.h>
//#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

void dal_channel_create(DalChannel *channel, int appID, int channelID, char *name) {

  /* copying channel name, for debugging purpose */  
  strncpy(channel->name, name, sizeof(channel->name));
  channel->name[sizeof(channel->name) - 1] = 0;

  channel->app_id = appID;
  channel->chan_id = channelID;
  channel->non_blocking = 0;
  channel->is_local = 0;

  if(!strncmp(name, "c_event", 7)) {
    channel->non_blocking = 1;
  }

  //#ifdef DAL_USE_DNP
  if(channel->is_local == 0) {
    channel->fd = open("/devices/dnp/rdma", O_RDWR); 
    if(channel->fd == -1) {
      dna_printf("error: creating channel %s\n", name);
      return;
    }
    if(ioctl(channel->fd, DNP_CONNECT, channelID) < 0) {
      dna_printf("error: setting channel id ... channel=%s\n", name);
      close(channel->fd);
      return;
    }
    //dna_printf("channel %s mapped on rdma/%d, fd=%d\n", name, channelID, channel->fd);
    //#else
  } else {
    channel->size = 64;
    channel->buffer = kernel_malloc(channel->size, false);
    if(channel->buffer == NULL) {
      printf("error: memory allocation failed (channel buffer) ...\n");
      return;
    }
    channel->start = 0;
    channel->end = 0;
    dna_printf("channel %s created\n", name);
    //#endif
  }
}

