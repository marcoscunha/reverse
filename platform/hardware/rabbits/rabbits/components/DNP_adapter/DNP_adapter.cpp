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

#include <dnp.h>

#define DEBUG_DNP_ADAPTER

#ifdef DEBUG_DNP_ADAPTER
#define DPRINTF(fmt, args...)                               \
	 do { printf("[%s]dnp_ad: " fmt, name(), ##args); } while (0)
#define DCOUT if (1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if (0) cout
#endif

#define EPRINTF(fmt, args...)                               \
	 do { fprintf(stderr, "[%s]dnp_ad: " fmt, name(), ##args); } while (0)


#define NOC_SOCK 0
#define C_SOCK   7


DNP_adapter::DNP_adapter(sc_module_name _name, int x, int y, int z,
						 sc_clock& clock, sc_signal<bool>& reset):
	 sc_module(_name),
	 m_dnp_C_isocket("C_isocket"), m_dnp_C_tsocket("C_tsocket"),
	 m_dnp_NoC_isocket("NoC_isocket"), m_dnp_NoC_tsocket("NoC_tsocket"),
	 m_dnp("dnp_dev"),
	 m_x(x), m_y(y), m_z(z)
{

	 slave = new DNP_slave_device("s0");
	 master0 = new DNP_master_device("m0");
	 master1 = new DNP_master_device("m1");

	 m_dnp_C_tsocket.register_b_transport(this, &DNP_adapter::b_transport);
	 m_dnp_NoC_tsocket.register_b_transport(this, &DNP_adapter::b_transport);

	 m_dnp.reset_hw(reset);
	 m_dnp.clock(clock);


	 /*
	  * Connect the external interfaces (NoC and C)
	  */
	 m_dnp_C_isocket.bind(m_dnp.interT_tsockets[C_SOCK]);
	 m_dnp.interT_isockets[C_SOCK].bind(m_dnp_C_tsocket);

	 m_dnp_NoC_isocket.bind(m_dnp.interT_tsockets[NOC_SOCK]);
	 m_dnp.interT_isockets[NOC_SOCK].bind(m_dnp_NoC_tsocket);
	
	 /*
	  * Connect to internal BUS
	  */
	 m_dnp.master_isocket_M0.bind(master0->target_export);
	 m_dnp.master_isocket_M1.bind(master1->target_export);
	 slave->_initiator_port.bind(m_dnp.slave_tsocket);

 
	 irq = &m_dnp.interrupt;
}

DNP_adapter::~DNP_adapter()
{

}

int DNP_adapter::port_num(dimension_t dim, direction_t dir)
{
	return 1+dim*2+dir;
}

direction_t DNP_adapter::invert_dir(direction_t dir)
{
	return dir==DIR_P?DIR_M:DIR_P;
}

void DNP_adapter::link_to(DNP_adapter* o, dimension_t dim, direction_t dir){
	int my_port = port_num(dim, dir);
	int other_port = port_num(dim, invert_dir(dir));
	link_to(my_port, o, other_port);
}

void DNP_adapter::link_to(int my_port, DNP_adapter* o, int other_port)
{
	if (!o)
	{
		cerr << "null pointer to other DNP-with-link" << endl;
		exit(0);
	}

	o->m_dnp.interT_isockets[other_port].bind(m_dnp.interT_tsockets[my_port]);
	m_dnp.interT_isockets[my_port].bind(o->m_dnp.interT_tsockets[other_port]);

}

void DNP_adapter::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
	 fprintf(stderr, "Unhandled message\n");
}

master_device *
DNP_adapter::get_master(int id)
{
	 switch(id){
	 case 0:
		  return master0;
	 case 1:
		  return master1;
	 default:
		  return NULL;
	 }
}

slave_device *
DNP_adapter::get_slave(int id){
	 if(id != 0)
		  return NULL;

	 return slave;
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
