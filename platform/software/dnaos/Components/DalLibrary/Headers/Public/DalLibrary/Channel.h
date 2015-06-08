#ifndef _DAL_CHANNEL_H
#define _DAL_CHANNEL_H_

//#define DAL_USE_DNP // use dnp for dal channels (instead of ring buffer)

typedef struct s_dal_channel {
  char name[128];
  int app_id;
  int chan_id;
  int non_blocking;
  int is_local;
//#ifdef DAL_USE_DNP
  int16_t fd;
//#endif
  unsigned int size;
  unsigned char * buffer;
  volatile unsigned int end;
  volatile unsigned int start;
} DalChannel;

void dal_channel_create(DalChannel *channel, int appID, int channelID, char *name);
dal_status_t dal_read(void *port, void *data, int32_t n, DALProcess *p);
dal_status_t dal_write(void *port, void *data, int32_t n, DALProcess *p);

#endif
