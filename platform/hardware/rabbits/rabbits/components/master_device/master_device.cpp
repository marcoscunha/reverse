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
#include <master_device.h>

#define QEMU_NOC_ADDRESS_DIFFERENCE 0x00000000

static unsigned char s_operation_mask_be[] =
    {0xDE, 0x01, 0x03, 0xDE, 0x0F, 0xDE, 0xDE, 0xDE, 0xFF};


master_device::master_device (sc_module_name module_name) :
    sc_module (module_name)
{
    SC_THREAD (response_thread);
}

master_device::~master_device (void)
{
}

/**
 * Thread responsible to get response from buffer over interconnection
 * Name: response_thread
 * Function: manage response from slave
 */
void master_device::response_thread ()
{
    vci_response                    resp;
    uint8_t                         be, ofs;
    int                             nbytes;

    for (;;)
    {
        get_port->get (resp); /// get response information from slave

        if (resp.rsrcid != m_node_id)
        {
            cout << "[Error: " << name () <<  " (id = " << m_node_id
                 << ") has received a response for " <<  resp.rsrcid
                 << "]" << endl;
            continue;
        }

        if (resp.reop == 0)
        {
            cout << "[Error: " << name () <<  " (id = " << m_node_id << 
                ") has received a response without EOP set]" << endl;
            continue;
        }

        if (resp.rerror)
            cout << "[Error in " << name () <<  endl << resp << "]" << endl;

          /// ignore non-enabled bytes
        be = resp.rbe;
        ofs = 0;
        if (be)
        {
            nbytes = 0;

            while (!(be & 0x1))
            {
                ofs++;
                be >>= 1;
            }

            while (be & 0x1)
            {
                nbytes++;
                be >>= 1;
            }
        }
          /// receive response: get enabled bytes from slave data
#ifdef TRACE_EVENT_ENABLED
        rcv_rsp (resp.rtrdid, &resp.rdata[ofs], resp.rerror, 0, (hwe_cont*)resp.hwe_cont_rsp);
#else
        rcv_rsp (resp.rtrdid, &resp.rdata[ofs], resp.rerror, 0);
#endif
    }
}

/**
 * @brief Function to send master request over interconnection
 *
 * @param tid Slave device ID
 * @param addr Address to acessed on peripheral device
 * @param data Information to send through inteconnection
 * @param nbytes number of bytes
 * @param bWrite 1 for Write or 0 for Read operations
 * @param hwe_ref Refecence of event that requires information
 */
#ifdef TRACE_EVENT_ENABLED
void master_device::send_req (unsigned char tid, uint32_t addr,
    uint8_t *data, uint8_t nbytes, bool bWrite, hwe_cont* hwe_cont_ref)
#else
void master_device::send_req (unsigned char tid, uint32_t addr,
    uint8_t *data, uint8_t nbytes, bool bWrite)
#endif
{
    int                     i;
    uint8_t                 ofs, mask_be, plen;
    vci_request             req;

    if (nbytes == 0) {
        fprintf(stderr, "%s - Error: nbytes is 0 in %s", __PRETTY_FUNCTION__, name());
        // return -1;
    }

    /// compute be and data from nbytes.
    plen = (nbytes + 3) >> 2;
    if (nbytes > 4)
        nbytes = 4;
    ofs = addr & 0x000000007;
    addr &= 0xFFFFFFF8;
    mask_be = s_operation_mask_be[nbytes] << ofs;

    req.cmd     = (bWrite ? CMD_WRITE : CMD_READ);
    req.be      = mask_be;
    req.address = addr - QEMU_NOC_ADDRESS_DIFFERENCE;
    req.trdid   = tid;
    req.srcid   = m_node_id;
    req.plen    = plen;
    req.eop     = 1;
    #ifdef TRACE_EVENT_ENABLED
    req.hwe_cont_ref = (uintptr_t)hwe_cont_ref;
    #endif

    memset (&req.wdata, 0, 8);

    if (bWrite)
    {
        for (i = 0; i < nbytes; i++)
            req.wdata[i + ofs] = data[i];
    }

      /// place request information at master output port
    put_port->put (req);
    //return 0;
}

/**
* Name: get_master
* Function: returns a pointer to slave_device structure. 
*           Used to standardize calls to slaves.
*/
master_device* master_device::get_master(int id)
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
