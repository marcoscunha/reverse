/*
 *  Copyright (c) 2013 TIMA Laboratory
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

#include <unistd.h>
#include <string.h>

#include <system_init.h>
#include <systemc.h>

#include <hwetrace.h>
#include <bunny.h>

int sc_main (int argc, char ** argv)
{
    init_struct is;

    sc_report_handler::set_actions (SC_INFO, SC_DO_NOTHING);

    memset (&is, 0, sizeof (init_struct));
    is.cpu_family = "arm";
    is.cpu_model = NULL;
    is.kernel_filename = NULL;
    is.initrd_filename = NULL;
    is.no_cpus = 1;
    is.ramsize = 1024 * 1024 * 1024;
#ifdef TRACE_EVENT_ENABLED
    is.trace_filename = NULL;
#endif
    parse_cmdline (argc, argv, &is);

    if(is.block_device == NULL)
        is.block_device = "ice_age_256x144_411.mjpeg";

#ifdef TRACE_EVENT_ENABLED
    if( is.trace_filename == NULL)
       hwetrace_open ("RABBITS");
    else
       hwetrace_open (is.trace_filename);
           
#endif

    bunny bunny("bunny", &is);

    sc_start ();

    fprintf(stderr, "Stopping Platform\n");

    return 0;
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
