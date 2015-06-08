#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include <Private/Dispatcher.h>
#include <Private/Decoder.h>
#include <Private/Serializer.h>

#include <PosixThreads/PosixThreads.h>
#include <KahnProcessNetwork/KahnProcessNetwork.h>
#include <SoclibFramebufferDriver/Driver.h>
#include <Processor/Cache.h>

#ifndef QEMU_ADDR_BASE

#define QEMU_ADDR_BASE                              0x82000000
#define SYSTEMC_SHUTDOWN                            0x0004

#define EXIT_SIM_SUCCESS \
        (*((volatile uint32_t *)(QEMU_ADDR_BASE + SYSTEMC_SHUTDOWN)) = 1)
#define EXIT_SIM_FAILURE \
        (*((volatile uint32_t *)(QEMU_ADDR_BASE + SYSTEMC_SHUTDOWN)) = 0)
#endif


int main (void)
{
/*uint32_t volatile i, j, k, n;
    k = 0; n = 0;
    for( i =0, j=0; i < 100000; i++, j=j+2){
        k = n*2;
        n++;
    }
    k =0;
   i = k;
   while(i!= 1000) i++; // XXX: Just to execute some instructions before shutdown
   EXIT_SIM_SUCCESS;
*/

  kpn_status_t status;

	kpn_channel_t dispatcher_channel[NB_DECODER + 1];
	kpn_channel_t decoder_channel[NB_DECODER][2];
	kpn_channel_t serializer_channel[NB_DECODER + 1];

  pthread_t dispatcher_thread, decoder_thread[NB_DECODER],
            serializer_thread;

  /*
   * Create a channel connected to the input movie
   */

	status = kpn_channel_create ("/devices/disk/simulator/0",
      0x10000, & dispatcher_channel[0]);

  if (status == KPN_NO_ENTRY)
  {
    printf ("Error: Cannot open source file.\r\n");
    abort ();
  }

  /*
   * Create a channel connected to the framebuffer 
   */

  kpn_channel_create ("/devices/video/simulator/0", 0, & serializer_channel[0]);

  if (status == KPN_NO_ENTRY)
  {
    printf ("Error: Cannot open framebuffer.\r\n");
    abort ();
  }

  ioctl (serializer_channel[0] -> fd, FB_SET_AUTOREWIND, (void *) true);

  /*
   * Create the channel between the dispatcher and the decoder
   */

  for (int32_t i = 0; i < NB_DECODER; i += 1)
  {
    char device_name[48];

    sprintf (device_name, "/devices/channel/rendezvous/%ld", 2 * i);
    kpn_channel_create (device_name, 0x3000, & dispatcher_channel[i + 1]);
    kpn_channel_create (device_name, 0x3000, & decoder_channel[i][0]);

    sprintf (device_name, "/devices/channel/rendezvous/%ld", 2 * i + 1);
    kpn_channel_create (device_name, 0, & serializer_channel[i + 1]);
    kpn_channel_create (device_name, 0, & decoder_channel[i][1]);
  }

  /*
   * Create the threads
   */

  printf ("Create dispatcher thread.\r\n");
  pthread_create (& dispatcher_thread, NULL,
      (pthread_func_t) dispatcher, dispatcher_channel);

  for (int32_t i = 0; i < NB_DECODER; i += 1)
  {
//    DCACHE_FLUSH(&i,sizeof(int32_t));  
    printf ("Create decoder thread %ld.\r\n", i);
    pthread_create (& decoder_thread[i], NULL,
        (pthread_func_t) decoder, decoder_channel[i]);
  }

  printf ("Create serializer thread.\r\n");
  pthread_create (& serializer_thread, NULL,
      (pthread_func_t) serializer, serializer_channel);

  /*
   * Joining the threads altogether
   */

  pthread_join (dispatcher_thread, NULL);
	return 0;
}
