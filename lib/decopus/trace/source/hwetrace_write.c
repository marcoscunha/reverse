
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "events/hwe_tools.h"

/****************************************************
 * 'hwe handler' which dumps all events into a file *
 ****************************************************/

struct comp_data_t {
   hwe_id_ind_t  lastid;
};

//#define USE_STAGE_GO
//#define USE_STAGE_RET
#define COMP_DATA_T struct comp_data_t
//#define EVENT_DATA_T struct event_data_t
#include "hwe_handle_main.h"

static int fd = -1;
#define BUFFERSIZE 1000000
static void *buffer = NULL;
static size_t bufsize = 0;

/*
 * buffer flush to the file
 */
static void buffer_flush()
{
   write(fd, buffer, bufsize);
   bufsize = 0;
}

/*
 * write 1 event into the buffer
 */
static void buffer_event(event_t *e)
{
   hwe_cont *hwe = e->hwe;
   comp_t *c = e->comp;
	hwe_id_t ids[HWE_REF_MAX];

   // write main event into the buffer
   {
		for (unsigned i = 0; i < hwe_getnref(&hwe->common); i += 1) {
			ids[i] = hwe_ref2id(hwe_getref(&hwe->common, i));
		}

      size_t size = hwe_sizeof(hwe);
      if (bufsize + size + HWE_ID_SIZEOF > BUFFERSIZE)
         buffer_flush();

      // if necessary insert a HWE_ID
      if (!hwe_head_rid_compute(&hwe->common, c->data.lastid)) {
         hwe_id_write(&hwe->common, buffer + bufsize);
         bufsize += HWE_ID_SIZEOF;
      }

      hwe_write(hwe, &ids[0], buffer + bufsize);
      bufsize += size;
      
      //update id
      c->data.lastid = hwe->common.id.index;
   }
   
   // write additionals containers
   while ((hwe = ((hwe_cont *) hwe->common.refnext))) {
		for (unsigned i = 0; i < hwe_getnref(&hwe->common); i += 1) {
			ids[i] = hwe_ref2id(hwe_getref(&hwe->common, i));
		}
      
      size_t size = hwe_sizeof(hwe);
      if (bufsize + size > BUFFERSIZE)
         buffer_flush();

      //set rid to 0 (ie: same event)
      hwe_head_rid_zero(&hwe->common);

      hwe_write(hwe, &ids[0], buffer + bufsize);
      bufsize += size;
   }
}

/*
 * init
 */
void process_init(const char *tracename, int nopt, char * const opt[])
{
   //build filename
   char fname[50];
   strncpy(fname, tracename, 30);
   fname[30] = '\0';
   strcat(fname, ".hwe");

   //open file
   fd = creat(fname, S_IRUSR | S_IWUSR);

   //init buffer
   bufsize = 0;
   buffer = malloc(BUFFERSIZE);
}

/*
 * stop
 */
void process_stop()
{
   //flush buffer, close & free things
   buffer_flush();
   close(fd);
   free(buffer);
}

/*
 * start
 */
void process_start()
{
   ;
}

/*
 * info
 */
void process_component(comp_t *comp, event_t *e)
{
   //put the event into the file
   buffer_event(e);
   //register the callback which will write events issued by this component
   set_cb_init(comp, buffer_event);
}

