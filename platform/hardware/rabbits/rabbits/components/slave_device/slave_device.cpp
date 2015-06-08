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

#include <cfg.h>
#include <slave_device.h>

slave_device::slave_device (sc_module_name module_name) : sc_module (module_name)
{
    m_write_invalidate = false;
    m_bProcessing_rq = false;
    SC_THREAD (request_thread);
}

slave_device::~slave_device ()
{
}

/**
 * Thread to get the requets from interconnection 
 * Name: request_thread
 * Function: manage request from master                                        *
 */
void slave_device::request_thread ()
{
    uint8_t                 be, cmd;
    uint32_t                addr;

    while (1)
    {
        get_port->get (m_req);  /// get request information from master

        if(m_bProcessing_rq)
        {
            fprintf(stdout, "Recieved a request while processing previous one: drop it\n");
            continue;
        }

        m_bProcessing_rq = true;

        addr   = m_req.address;
        be     = m_req.be;
        cmd    = m_req.cmd;

        m_rsp.rsrcid   = m_req.srcid;
        m_rsp.rtrdid   = m_req.trdid;
        m_rsp.reop     = 1;
        m_rsp.rbe      = be;
#ifdef TRACE_EVENT_ENABLED
        m_rsp.hwe_cont_rsp  = m_req.hwe_cont_ref;
#endif

          /// receive request
        switch (cmd)
        {
        case CMD_WRITE:
            this->rcv_rqst (addr, be, m_req.wdata, true);
            break;
        case CMD_READ:
            this->rcv_rqst (addr, be, m_rsp.rdata, false);
            break;
        default:
            cerr << "Unknown command" << endl;
        } //switch (cmd)
    } // while(1)
}

/**
 * Funtion responsable to send response over interconnection 
 * - bErr
 */
void
slave_device::send_rsp (bool bErr)
{
    if (m_write_invalidate && m_req.cmd == CMD_WRITE){
#ifdef TRACE_EVENT_ENABLED
        m_subsys->invalidate_address(m_req.initial_address,
            m_req.slave_id, m_req.initial_address, m_req.srcid,
            (hwe_cont*)m_req.hwe_cont_ref);
#else
        m_subsys->invalidate_address(m_req.initial_address,
            m_req.slave_id, m_req.initial_address, m_req.srcid);
#endif
    }

    m_rsp.rerror = bErr;
    put_port->put (m_rsp);

    m_bProcessing_rq = false;
}
 
/**
* Name: get_slave
* Function: returns a pointer to slave_device structure. 
*           Used to standardize calls to slaves.
*/
slave_device* slave_device::get_slave(int id)
{
    if(id != 0) return NULL;
    return(this);
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
