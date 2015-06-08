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

#define DEBUG_DNP_SLAVE

#ifdef DEBUG_DNP_SLAVE
#define DPRINTF(fmt, args...)                               \
	 do { printf("[%s]dnp_sl: " fmt, name(), ##args); } while (0)
#define DCOUT if (1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if (0) cout
#endif

#define EPRINTF(fmt, args...)                               \
	 do { fprintf(stderr, "[%s]dnp_sl: " fmt, name(), ##args); } while (0)



DNP_slave_device::DNP_slave_device(sc_module_name _name): 
	 slave_device(_name), _initiator_port("dnp_slave_socket")
{

}

DNP_slave_device::~DNP_slave_device()
{}


void DNP_slave_device::read (uint32_t offset, uint32_t *data, bool &bErr)
{
	 sc_time delay = sc_time(0, SC_NS);
	 tlm::tlm_generic_payload req;

	 bErr = false;
	 
	 req.set_command(tlm::TLM_READ_COMMAND);
	 req.set_address(offset);
	 req.set_data_length(4);
	 req.set_data_ptr((unsigned char *)data);
	 
	 _initiator_port->b_transport(req, delay);
	 
	 if (req.get_response_status() != tlm::TLM_OK_RESPONSE){
		  DPRINTF("R: error\n");
		  bErr = true;
	 }
	 
}

void DNP_slave_device::write (uint32_t offset, uint32_t *data, bool &bErr)
{

	 sc_time delay = sc_time(0, SC_NS);
	 tlm::tlm_generic_payload req;

	 bErr = false;

	 req.set_address(offset);
	 req.set_data_ptr((unsigned char *) data);
	 req.set_data_length(4);
	 req.set_command(tlm::TLM_WRITE_COMMAND);

	 _initiator_port->b_transport(req, delay);

	 if(req.get_response_status() != tlm::TLM_OK_RESPONSE){
		  DPRINTF("W: error\n");
		  bErr = true;
	 }
}

void DNP_slave_device::rcv_rqst (uint32_t ofs, uint8_t be,
										uint8_t *data, bool bWrite)
{

	 /*
	  * DNP Slave is a 32bit interface.
	  * So lets get rid of that 64 bit packet now.
	  */

	 bool      bErr  = false;
	 uint32_t *pdata = (uint32_t *)data;
	 uint32_t *ldata = pdata;
	 uint32_t  lofs  = ofs;

	 switch(be){
	 case 0xF0:
		  lofs += 4;
		  ldata++;
	 case 0x0F:

		  if(bWrite){
			   DPRINTF("W @ 0x%08x -- 0x%08x\n", lofs, *ldata);
			   this->write(lofs, ldata, bErr);
			   
		  }else{
			   this->read(lofs, ldata, bErr);
			   DPRINTF("R @ 0x%08x -- 0x%08x\n", lofs, *ldata);

		  }
		  break;
	 default:
		  bErr = true;
	 }

	 send_rsp(bErr);
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
