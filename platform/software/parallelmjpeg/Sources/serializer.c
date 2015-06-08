/*
 * Copyright (C) 2007 TIMA Laboratory
 *
 * Author(s):
 *   Patrice GERIN, patrice.gerin@imag.fr
 *   Xavier GUERIN, xavier.guerin@imag.fr
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#include <Private/Serializer.h>
#include <Processor/Processor.h>

#define MAX_FRAMES 30
#define QEMU_ADDR_BASE                              0x82000000
//#define EXIT_STATUS                                 0x0090
#define SYSTEMC_SHUTDOWN                            0x0004
#define LOG_END_OF_IMAGE                            0x0050

#define EXIT_SIM_SUCCESS \
     (*((volatile uint32_t *)(QEMU_ADDR_BASE + SYSTEMC_SHUTDOWN)) = 1)
#define EXIT_SIM_FAILURE \
     (*((volatile uint32_t *)(QEMU_ADDR_BASE + SYSTEMC_SHUTDOWN)) = 0)

#define REG_LOG_END_OF_IMAGE \
     (*((volatile uint32_t *)(QEMU_ADDR_BASE + LOG_END_OF_IMAGE)) = 1)

/*
 * The serializer function
 */

int32_t serializer (kpn_channel_t c[NB_DECODER + 1])
{
  uint8_t * buffer = 0;
  int32_t next_decoder = 0;
  bigtime_t old, new;
  
  uint32_t volatile frames = 1;

  /*
   * Allocate a dummy sized buffer
   */

  buffer = malloc (256 * 144 * 2);

  /*
   * Parse the flows
   */

  cpu_timer_get (0, & old);

  while (true)
  {    
    kpn_channel_read (c[next_decoder + 1], buffer, 256 * 144 * 2);
    kpn_channel_write (c[0], buffer, 256 * 144 * 2);
    next_decoder = (next_decoder + 1) % NB_DECODER;

    cpu_timer_get (0, & new);
    IPRINTF ("1 frame in %ld ns\r\n", new - old);
    old = new;

//    REG_LOG_END_OF_IMAGE;
    if (frames == MAX_FRAMES){
        unsigned volatile i = 0;
        while(i!= 10000) i++;
        EXIT_SIM_SUCCESS;
    }
    frames++;

  }
}

