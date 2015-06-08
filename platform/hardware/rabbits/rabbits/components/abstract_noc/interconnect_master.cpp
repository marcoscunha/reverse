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

#include <interconnect_master.h>
#include <interconnect_slave.h>
#include <mwsr_ta_fifo.h>

/**
* Name: interconnect_master
* Function: interconnect the selected master with slaves (adress mapping)
* Inputs: name: master name
*         parent: master parent
*         srcid: master id
*/
interconnect_master::interconnect_master (sc_module_name name, interconnect *parent, int srcid)
: sc_module (name)
{
    m_srcid = srcid;
    m_parent = parent;

    /// create request and response queues
    m_queue_responses = new mwsr_ta_fifo<vci_response> (8);
    m_queue_requests = new mwsr_ta_fifo<vci_request> (8);

    SC_THREAD (dispatch_requests_thread);
}

/**
* Name: ~interconnect_master
* Function: delete master maps and fifos
*/
interconnect_master::~interconnect_master ()
{

    if (m_queue_responses)
        delete m_queue_responses;

    if (m_queue_requests)
        delete m_queue_requests;
}

///put interface
void interconnect_master::put (vci_request &req)
{
    m_queue_requests->Write (req);
}

///get interface
void interconnect_master::get (vci_response &rsp)
{
    rsp = m_queue_responses->Read ();
}


/**
* Name: dispatch_requests_thread
* Function: read a master 'request' queue message. Extract an adress from the
*           message and select a slave (if it exists) that corresponds. If a
*           slave was found, send the request to the slave
*/
void interconnect_master::dispatch_requests_thread ()
{
    uint32_t                            addr;
    int                                 i, slave_id;
    vci_request                         req;
    interconnect_slave                  *slave;
    interconnect_address_map_elt_t     *map;
    while (1)
    {
          /// get request information from fifo
        req = m_queue_requests->Read ();

        wait (3, SC_NS);

          /// check if address is inside an area defined in map file
        addr = req.address;
        map = NULL;
        for (i = 0; i < m_mmap->n_elts; i++){
            map = &m_mmap->elts[i];
            if (addr >= map->begin_address && addr < map->end_address)
                break;
        }
        if (i == m_mmap->n_elts)
        {
              /// selected address is outside.
            printf ("Error (masterid=%d): Cannot map the address 0x%x to a slave!\n",
                m_srcid, addr);
            exit (1);
        }
        slave_id = map->slave_id; /// i: slave index
        slave = m_parent->get_slave (slave_id);

          /// check is slave exists
        if (slave == NULL)
        {
            printf ("Error (masterid=%d): Cannot find the slaveid %d!\n",
                m_srcid, slave_id);
            exit (1);
        }

          /// a slave was found. Send the request to the slave 
        req.initial_address = req.address;
        req.slave_id = slave_id;
        req.address = addr - map->begin_address;

        slave->add_request (req);
    }
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
