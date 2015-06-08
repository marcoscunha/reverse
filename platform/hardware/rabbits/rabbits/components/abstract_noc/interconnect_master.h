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

#ifndef _INTERCONNECT_MASTER_H_
#define _INTERCONNECT_MASTER_H_

#include <interconnect.h>
#include <mwsr_ta_fifo.h>

template <typename ITEM>
class mwsr_ta_fifo;

class interconnect_master :
    public sc_module, public VCI_PUT_REQ_IF, public VCI_GET_RSP_IF
{
public:
    SC_HAS_PROCESS (interconnect_master);
    interconnect_master (sc_module_name name, interconnect *parent, int srcid);
    ~interconnect_master ();

public:
    //get interface
    virtual void get (vci_response&);
    //put interface
    virtual void put (vci_request&);

public:
    inline void add_response (vci_response& rsp)
    {
        m_queue_responses->Write (rsp);
    }

    inline int get_linear_address (
        int slave_id, uint32_t offset_slave, uint32_t &addr)
    {
        interconnect_address_map_elt_t *map;
        int                             i;

        for (i = 0; i < m_mmap->n_elts; i++)
        {
            map = &m_mmap->elts[i];
            if (map->slave_id == slave_id)
            {
                addr = map->begin_address + offset_slave;
                if (addr >= map->begin_address && addr < map->end_address)
                    return 1;
            }
        }

        return 0;
    }

    inline int get_slave_id_for_mem_addr (uint32_t &addr)
    {
        interconnect_address_map_elt_t *map;
        int                             i;

        for (i = 0; i < m_mmap->n_elts; i++)
        {
            map = &m_mmap->elts[i];
            if (addr >= map->begin_address && addr <= map->end_address)
            {
                addr = addr - map->begin_address;
                return map->slave_id;
            }
        }

        fprintf(stderr, "%s : Error: 0x%x bad address required!\n", __PRETTY_FUNCTION__, addr);
        return -1;
    }

private:
    //thread
    void dispatch_requests_thread ();

public:
    int                                 m_srcid;   /// master id
    interconnect                       *m_parent;  /// master parent
    interconnect_address_map_t         *m_mmap;
    mwsr_ta_fifo<vci_response>         *m_queue_responses;
    mwsr_ta_fifo<vci_request>          *m_queue_requests;
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
