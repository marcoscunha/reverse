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

#ifndef _MEM_DEVICE_H_
#define _MEM_DEVICE_H_

#include <slave_device.h>
#include <unordered_map>

#define MEM_WRITE true
#define MEM_READ  false

typedef struct {
    uint32_t    readers;
    uint32_t    writers;
    long int    fofs;
//    bool        shared;
}shared_t;

using namespace std;

class mem_device : public slave_device
{
public:
    mem_device (const char *_name, unsigned long _size, generic_subsystem *sub);
    virtual ~mem_device ();

public:
    /*
     *   Obtained from father
     *   void send_rsp (bool bErr);
     */
    virtual void rcv_rqst (uint32_t ofs, uint8_t be, uint8_t *data, bool bWrite);


    virtual unsigned char *get_mem () {return mem;}
    virtual unsigned long get_size () {return size;}

private:
    void write (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr);
    void read  (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr);
    void dump(void);

#ifdef TRACE_EVENT_ENABLED
    void tr_ack (uint8_t *data, uint32_t ofs, uint8_t be);

    void    create_mem_init(void);
    uint8_t mem_init(uint8_t state, uint32_t addr);
    void    analyse_mem_mod(uint32_t addr);
    bool    get_addr_width(uint32_t ofs, uint8_t be, uint32_t* addr, uint32_t *width);
    void    create_mem_shared(void);
    void    analyse_mem_shared(uint32_t addr, bool type);


#endif

private:
    unsigned long       size;
    unsigned char       *mem;

    #ifdef TRACE_EVENT_ENABLED
    hwe_port_t* hwe_mem_port;
    hwe_port_t* hwe_periph_port;
    std::unordered_map<uint32_t, uint64_t> mod_mem;
    std::unordered_map<uint32_t, shared_t> shared_mem;
    FILE *shared_fd; 

    #endif

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
