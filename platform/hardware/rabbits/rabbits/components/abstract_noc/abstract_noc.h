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

#ifndef _ABRSTACT_NOC_H
#define _ABRSTACT_NOC_H

#define MAP_FILES_DIR "./maps"
#include <cfg.h>
#include <stdint.h>
#include <systemc.h>

#ifdef TRACE_EVENT_ENABLED
#include <events/hwe_events.h>

#endif
namespace noc
{

    // B = 8
    // K = 8
    // N = 32
    // E = 1
    // Q = 1
    // F = 1
    // S = 14
    // P = 4
    // T = 4
    // W = 1


    class vci_request
    {
    public:
        uint32_t    address;        /**< Destination Address            */
        uint8_t     be;             /**< ?                              */
        uint8_t     cmd;            /**< ?                              */
        bool        contig;         /**< ?                              */
        uint8_t     wdata[8];       /**< Data                           */
        bool        eop;            /**< ?                              */
        bool        cons;           /**< ?                              */
        uint8_t     plen;           /**< Package length                 */
        bool        wrap;           /**< ?                              */
        bool        cfixed;         /**< ?                              */
        bool        clen;           /**< ?                              */
        uint16_t    srcid;          /**< Source Identification          */
        uint8_t     trdid;          /**< ?                              */
        uint8_t     pktid;          /**< Packet Identification          */

        uint32_t    initial_address;/**< ?                              */
        int         slave_id;       /**< Slave identification           */
        #ifdef TRACE_EVENT_ENABLED
        uintptr_t hwe_cont_ref;      /**< Trace source identification    */
        #endif
    };
    enum {
        CMD_NOP,
        CMD_READ,
        CMD_WRITE,
        CMD_LOCKED_READ,
        CMD_STORE_COND = CMD_NOP,
    };

/*  The request packet structure contains a control and a data part.

    The control part is composed of the

    - operation code (some bits represent the operation type and some bits represent the number of bytes implied in the operation),
    - a bitmap field specifying which bytes in the data part contain useful information,
    - the destination address, the transaction identifier,
    - the source master identifier
    - a field that specifies whether the packet is the last one in the transaction.

*/
    class vci_response
    {
    public:
        uint8_t   rdata[8];  // Response data
        bool      reop;      //< bit End of OPeration
        bool      rerror;    //< Response Error
        uint16_t  rsrcid;    //< Response Source Identification
        uint8_t   rtrdid;    //< ?
        uint8_t   rpktid;    //< Response Package Identification

        // **************
        // Extra field
        uint8_t   rbe;       //< ?
        #ifdef TRACE_EVENT_ENABLED
        uintptr_t   hwe_cont_rsp;      /**< Trace source identification    */
        #endif
    };

    template <typename T>
        class tlm_blocking_put_if : public sc_interface
    {
    public:
        virtual void put (T&) = 0;
    };

    template <typename T>
        class tlm_blocking_get_if : public sc_interface
    {
    public:
        virtual void get (T&) = 0;
    };


    typedef tlm_blocking_put_if<vci_request>  VCI_PUT_REQ_IF;
    typedef tlm_blocking_put_if<vci_response> VCI_PUT_RSP_IF;

    typedef tlm_blocking_get_if<vci_response> VCI_GET_RSP_IF;
    typedef tlm_blocking_get_if<vci_request>  VCI_GET_REQ_IF;

}

using namespace noc;

ostream &operator<<(ostream& output, const vci_request& value);
ostream &operator<<(ostream& output, const vci_response& value);

#endif /* _ABRSTACT_NOC_H */

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
