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

#ifndef __DNP_ADAPTER_H__
#define __DNP_ADAPTER_H__

#include <tlm.h>
#include <master_device.h>
#include <slave_device.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <dnp.h>

class DNP_master_device: public master_device
{
public:
	DNP_master_device(const char * _name);
	~DNP_master_device();

public:
	virtual void rcv_rsp (uint8_t tid, uint8_t *data,
	                          bool bErr, bool bWrite);
	void master_cb( tlm::tlm_generic_payload &arg_Req, sc_time& delay);

public:
	tlm_utils::simple_target_socket<DNP_master_device> target_export;

private:
	uint8_t recvd_data[8];

public:
	uint8_t crt_trans_id;     // current transaction id.
	sc_event rsp_rcvd_ev;     // response received event.
};

class DNP_slave_device : public slave_device
{
public:
	DNP_slave_device(sc_module_name name);
	~DNP_slave_device();

public:
    virtual void rcv_rqst (uint32_t ofs, uint8_t be, uint8_t *data, bool bWrite);

private:
    void write (uint32_t ofs, uint32_t *data, bool &bErr);
    void read (uint32_t ofs, uint32_t *data, bool &bErr);

public:
	tlm_utils::simple_initiator_socket<DNP_slave_device>  _initiator_port;

};



enum dimension_t
{
	DIM_X = 0,
	DIM_Y = 1,
	DIM_Z = 2,
	NUM_DIMS = 3,
};
enum direction_t
{
	DIR_P,
	DIR_M
};

class DNP_adapter : public sc_module
{

public:
	 DNP_adapter(sc_module_name name, int x, int y, int z,
				 sc_clock& clock, sc_signal<bool>& reset);
	~DNP_adapter();

private:
	DNP_slave_device  *slave;
	DNP_master_device *master0;
	DNP_master_device *master1;


public:
    tlm_utils::simple_initiator_socket<DNP_adapter> m_dnp_C_isocket;
    tlm_utils::simple_target_socket<DNP_adapter>    m_dnp_C_tsocket;

    tlm_utils::simple_initiator_socket<DNP_adapter> m_dnp_NoC_isocket;
    tlm_utils::simple_target_socket<DNP_adapter>    m_dnp_NoC_tsocket;

	void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

	dnp              	m_dnp;
	int              	m_x;
	int              	m_y;
	int              	m_z;

public:

	sc_out<bool>      *irq;

	void link_to(int my_port, DNP_adapter* o, int other_port);

	void link_to(DNP_adapter* o, dimension_t dim, direction_t dir);
	direction_t invert_dir(direction_t dir);
	int port_num(dimension_t dim, direction_t dir);

	master_device *get_master(int id);
	slave_device  *get_slave(int id = 0);
};

#endif /* __DNP_ADAPTER_H__ */

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
