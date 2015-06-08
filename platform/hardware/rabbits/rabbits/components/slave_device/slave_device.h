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

#ifndef _SLAVE_DEVICE_H_
#define _SLAVE_DEVICE_H_

class slave_device;

#include <abstract_noc.h>
#include <generic_subsystem.h>

/*
 * TODO: look for better 
 */
#include <../../qemu/sc_qemu/rabbits/systemc_imports.h>

class slave_device : public sc_module
{
public:
    //constructor & destructor
    SC_HAS_PROCESS (slave_device);
    slave_device (sc_module_name module_name);
    virtual ~slave_device ();

    inline uint32_t get_node_id(void) { return m_node_id; };
    inline void     set_node_id(uint32_t id) { m_node_id = id; };

private:
    //sc threads
    void request_thread ();

public:
    void send_rsp (bool bErr);

    // Slave modules have to implement the recieve function ...
    virtual void rcv_rqst (uint32_t ofs, uint8_t be,
                           uint8_t *data, bool bWrite) = 0;
    virtual unsigned char *get_mem () {return 0;}
    virtual unsigned long get_size () {return 0;}

    slave_device *get_slave(int id = 0);

public:
    //ports
    sc_port<VCI_GET_REQ_IF>          get_port;
    sc_port<VCI_PUT_RSP_IF>          put_port;

    vci_response                     m_rsp;
    vci_request                      m_req;
    bool                             m_bProcessing_rq;
    bool                             m_write_invalidate;

    // invalidation facilities
    // valid only if m_write_invalidate = true
    generic_subsystem               *m_subsys;

private:
    uint32_t                         m_node_id;
};

#endif

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
