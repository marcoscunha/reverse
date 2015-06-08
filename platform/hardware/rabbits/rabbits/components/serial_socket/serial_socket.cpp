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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <serial_socket.h>
#include <cfg.h>

//#define DEBUG_DEVICE_SERIAL_SOCKET_SERIAL

#ifdef DEBUG_DEVICE_SERIAL_SOCKET_SERIAL
#define DPRINTF(fmt, args...)                                           \
    do { fprintf(stderr, "DEBUG serial_socket_device: " fmt , ##args); } while (0)
#define DCOUT if (1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if (0) cout
#endif

#define SERIAL_SOCKET_INT_READ        1

int sock_server = -1;
int sock_data = -1;

#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/fcntl.h>
#include <errno.h>

int unix_socket_bind(char * path)
{
    int sock;

    printf("unix socket: creating server socket on %s\n", path);
    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket() failed");
        exit (666);
    }
    printf("unix socket: socket created : %d\n", sock);

    struct      sockaddr_un address;

    unlink (path);
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    int         address_length = sizeof (address.sun_family) + strlen(address.sun_path);
    printf("unix socket: binding %d\n", sock);
    if (bind(sock, (struct sockaddr *) &address, address_length) < 0)
    {
        perror("bind() failed");
        exit (666);
    }

    if (listen(sock, 5) < 0)
    {
        perror ("listen() failed");
        exit (666);
    }

    return sock;
}

int unix_socket_accept (int sock)
{
    int client;
    struct sockaddr_un address;
    socklen_t length = sizeof (address);

    client = accept(sock, (struct sockaddr *) &address, &length);
    if (client == -1 && errno != EAGAIN)
    {
        perror("unix_socket_accept");
    }

    return client;
}

void serial_socket_device::read_thread()
{
    int pos;
    unsigned char ch;

    while (1)
    {
        wait(10, SC_US);

        if (sock_data == -1)
        {
            sock_data = unix_socket_accept(sock_server);
            if (sock_data != -1)
            {
                printf("serial_socket: a debugger is now attached (sock=%d) \n", sock_data);
                fcntl(sock_data, F_SETFL, O_NONBLOCK);
            }
        }

        if (sock_data != -1) {

            int nread = ::read(sock_data, &ch, 1);
            if (nread == 0)
            {
                printf("serial_socket %d: connection closed.\n", sock_data);
                sock_data = -1;
            }
            else if (nread < 0)
            {
                if (errno != EAGAIN)
                {
                    perror("read error");
                    sock_data = -1;
                }
                nread = 0;
            }
            else
            {
                //printf("read char: %c\n", ch);

                while (state.read_count == SERIAL_SOCKET_READ_BUF_SIZE)
                    wait(evRead);

                pos = (state.read_pos + state.read_count) % SERIAL_SOCKET_READ_BUF_SIZE;
                state.read_buf[pos] = ch;
                state.read_count++;
                //end of read data

                state.int_level |= SERIAL_SOCKET_INT_READ;
                irq_update.notify();
            }
        }
    }
}

serial_socket_device::serial_socket_device(sc_module_name _name, const char * path)
: slave_device(_name)
{
    memset(&state, 0, sizeof (state));

    m_path = strdup(path);

    sock_server = unix_socket_bind(m_path);

    if (sock_server)
        printf("serial_socket: sock_server has descriptor %d\n", sock_server);

    fcntl(sock_server, F_SETFL, O_NONBLOCK); // for non-blocking accept

    SC_THREAD(read_thread);
    SC_THREAD(irq_update_thread);
}

serial_socket_device::~serial_socket_device()
{
    if (m_path)
        free(m_path);
}

void serial_socket_device::irq_update_thread()
{
    unsigned long flags;

    while (1)
    {
        wait(irq_update);

        flags = state.int_level & state.int_enabled;

        DPRINTF("%s - %s\n", __FUNCTION__, (flags != 0) ? "1" : "0");

        irq_line = (flags != 0);
    }
}

void serial_socket_device::write(uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr)
{
    uint8_t         ch;
    uint32_t        value;

    bErr = false;

    ofs >>= 2;
    if (be & 0xF0)
    {
        ofs++;
        value = * ((uint32_t *) data + 1);
    }
    else
        value = * ((uint32_t *) data + 0);

    if (ofs != 0)
        DPRINTF("%s to 0x%lx - value 0x%lx\n", __FUNCTION__, ofs, value);

    switch (ofs)
    {
        case 0:
        {//write data
            ch = data[0];
            //::write(sock_data, &ch, 1);

            if (sock_data == -1)
                return;

#define MAX_TRIES 10
            int trycount = 0;
            int nwrite = 0;
            do {
                nwrite = ::write(sock_data, &ch, 1);
                if (nwrite <= 0)
                {
                    if (errno != EAGAIN)
                    {
                        perror("errno: ");
                        exit(0);
                    }
                    trycount++;
                    if (trycount >= MAX_TRIES)
                    {
                        printf(" serial(dbg_send): wasted 1ms...\n");
                        trycount = 0;
                    }
                    usleep(100);
                }
            } while (nwrite <= 0);

            break;
        }
        case 1: //set int enable
            state.int_enabled = value;
            irq_update.notify();
            break;

        default:
                printf("Bad %s::%s ofs=0x%X, be=0x%X, data=0x%X-%X!\n",
                name(), __FUNCTION__, ofs, be,
                * ((uint32_t *) data + 0), * ((uint32_t *) data + 1));
            exit(1);
    }
}

void serial_socket_device::read (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr)
{
    uint32_t    c, *pdata;

    pdata = (uint32_t *) data;
    pdata[0] = 0;
    pdata[1] = 0;
    bErr = false;

    ofs >>= 2;
    if (be & 0xF0)
    {
        ofs++;
        pdata++;
    }

    switch (ofs)
    {
        case 0: //read char
            c = state.read_buf[state.read_pos];
            if (state.read_count > 0)
            {
                state.read_count--;
                if (++state.read_pos == SERIAL_SOCKET_READ_BUF_SIZE)
                    state.read_pos = 0;

                if (0 == state.read_count)
                {
                    state.int_level &= ~SERIAL_SOCKET_INT_READ;
                    irq_update.notify();
                }

                evRead.notify(0, SC_NS);
            }
            *pdata = c;
            break;

        case 1: //can write?
            *pdata = 1;
            break;

        case 2: //can read?
            *pdata = (state.read_count > 0) ? 1 : 0;
            break;

        case 3: //get int_enable
            *pdata = state.int_enabled;
            break;

        case 4: //get int_level
            *pdata = state.int_level;
            break;

        case 5: //active int
            *pdata = state.int_level & state.int_enabled;
            break;

        case 6:
            *pdata = 0;
            break;

        default:
                printf ("Bad %s::%s ofs=0x%X, be=0x%X!\n",
                name (), __FUNCTION__, (unsigned int) ofs, (unsigned int) be);
            exit (1);
    }

    if (ofs != 6)
        DPRINTF ("%s from 0x%lx - value 0x%lx\n", __FUNCTION__, ofs, *pdata);
}

void serial_socket_device::rcv_rqst (uint32_t ofs, uint8_t be,
    uint8_t *data, bool bWrite)
{
    bool bErr = false;

    if (bWrite)
        this->write (ofs, be, data, bErr);
    else
        this->read (ofs, be, data, bErr);

    send_rsp (bErr);
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
