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

#ifndef _INTERCONNECT_H
#define _INTERCONNECT_H

class interconnect;
class interconnect_master;
class interconnect_slave;
typedef struct interconnect_address_map interconnect_address_map_t;
typedef struct interconnect_address_map_elt interconnect_address_map_elt_t;

#include <abstract_noc.h>
#include <slave_device.h>
#include <master_device.h>

struct interconnect_address_map_elt {
    int                             slave_id;
    uint32_t                        begin_address;
    uint32_t                        end_address;
};

struct interconnect_address_map {
    int                             n_elts;
    interconnect_address_map_elt_t *elts;
};

class interconnect : public sc_module
{
public:
    SC_HAS_PROCESS (interconnect);
    interconnect (sc_module_name name, int nmasters, int nslaves);
    ~interconnect ();

public:
    void connect_master_64 (int devid, master_device *master, interconnect_address_map_t *addr_map);
    void connect_slave_64 (int devid, slave_device *slave);

public:
    inline interconnect_master* get_master (int id)
    {
        return m_masters[id];
    }

    inline interconnect_slave* get_slave (int id)
    {
        return m_slaves[id];
    }

    inline int get_nmasters () {return m_nMasters; }
    inline int get_nslaves () {return m_nSlaves; }

private:
    void internal_init ();

protected:
    int                                 m_nMasters; /// masters number
    interconnect_master                 **m_masters;
    int                                 m_nSlaves;  /// slaves number
    interconnect_slave                  **m_slaves;
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
