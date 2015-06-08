/*
 *  Copyright (c) 2010 TIMA Laboratory
 *
 *  This file is part of Rabbits.
 *
 *  Rabbits is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Rabbits is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Rabbits.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <DNP_adapter.h>

#define DEBUG_DNP_MASTER

#ifdef DEBUG_DNP_MASTER
#define DPRINTF(fmt, args...)                               \
	 do { printf("[%s]dnp_ma: " fmt, name(), ##args); } while (0)
#define DCOUT if (1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if (0) cout
#endif

#define EPRINTF(fmt, args...)                               \
	 do { fprintf(stderr, "[%s]dnp_ma: " fmt, name(), ##args); } while (0)


DNP_master_device::DNP_master_device(const char *_name):
	 master_device(_name)
{
	crt_trans_id = 0;
	target_export.register_b_transport(this, &DNP_master_device::master_cb);
}

DNP_master_device::~DNP_master_device()
{

}

void
DNP_master_device::rcv_rsp (uint8_t tid, uint8_t *data, bool bErr, bool bWrite)
{
	 if (tid != crt_trans_id){
		  EPRINTF ("Bad tid (%d / %d)\n", tid, crt_trans_id);
	 }

/* TODO */	 
	 if (bErr){

	 }else{
		  memcpy(recvd_data, (uint8_t *)data, 8); 
	 }
	 rsp_rcvd_ev.notify();
}

void DNP_master_device::master_cb( tlm::tlm_generic_payload &arg_Req, sc_time& delay)
{
	uint32_t  addr     = (uint32_t) arg_Req.get_address();
	uint32_t  len      = (uint32_t) arg_Req.get_data_length();
	uint8_t  *data_ptr = (uint8_t *) arg_Req.get_data_ptr();

	crt_trans_id ++ ; 

	memset(recvd_data, 0, sizeof(recvd_data));

	if (arg_Req.is_read()){
		 /*
		  * Read command
		  */
		 DPRINTF("R @ 0x%08x(%d) <%d>\n", addr, len, crt_trans_id);

		 for(uint32_t i = 0; i < len; i+=4){

#ifdef DEBUG_DNP_MASTER
			  uint32_t *p = (uint32_t *)recvd_data;
#endif			  
			  send_req(crt_trans_id, (addr + i), recvd_data, 4, false);
			  
			  wait(rsp_rcvd_ev);
			  memcpy((data_ptr + i), (uint32_t *)recvd_data, 4);

			  DPRINTF("R @ 0x%08x -- %08x \n", (addr + i), *p);
		 }
	}else{
		 /*
		  * Write command
		  */
		 DPRINTF("W @ 0x%08x(%d) <%d>\n", addr, len, crt_trans_id);

		 for (uint32_t i = 0; i < len; i+=4){

#ifdef DEBUG_DNP_MASTER
			  uint32_t *p = (uint32_t *)(data_ptr + i);
#endif
			  DPRINTF("W @ 0x%08x -- %08x\n", (addr + i), *p);
			  
			  send_req(crt_trans_id, (addr + i), (data_ptr + i), 4, true);
			  wait(rsp_rcvd_ev);
		}
	}
	arg_Req.set_response_status(tlm::TLM_OK_RESPONSE);
}

/*
 * Vim standard variables
 * vim:set ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
