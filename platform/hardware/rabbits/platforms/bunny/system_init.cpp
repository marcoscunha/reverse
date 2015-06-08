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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <system_init.h>
#include <sys/stat.h>
#include <errno.h>

#define BOARD_ID            2339
#define KERNEL_ARGS_ADDR    0x100
#define KERNEL_LOAD_ADDR    0x00010000
#define INITRD_LOAD_ADDR    0x00800000
#define HAS_ARG             0x0001

#ifndef O_BINARY
#define O_BINARY 0
#endif

extern unsigned long no_frames_to_simulate;

enum
{
    CMDLINE_OPTION_loops,
    CMDLINE_OPTION_ncpu,
	  CMDLINE_OPTION_cpu_family,
    CMDLINE_OPTION_cpu,
    CMDLINE_OPTION_ram,
    CMDLINE_OPTION_sram,
    CMDLINE_OPTION_kernel,
    CMDLINE_OPTION_initrd,
    CMDLINE_OPTION_gdb_port,
  	CMDLINE_OPTION_kernel_cmd,
    CMDLINE_OPTION_fb_uninit,
    CMDLINE_OPTION_block_dev,
    CMDLINE_OPTION_trace_name,

};

typedef struct
{
    const char      *name;
    int             flags;
    int             index;
} cmdline_option;

const cmdline_option cmdline_options[] = 
{
    {"ncpu",      HAS_ARG, CMDLINE_OPTION_ncpu},
    {"M",         HAS_ARG, CMDLINE_OPTION_cpu_family},
    {"cpu",       HAS_ARG, CMDLINE_OPTION_cpu},
    {"ram",       HAS_ARG, CMDLINE_OPTION_ram},
    {"sram",      HAS_ARG, CMDLINE_OPTION_sram},
    {"kernel",    HAS_ARG, CMDLINE_OPTION_kernel},
    {"initrd",    HAS_ARG, CMDLINE_OPTION_initrd},
    {"gdb_port",  HAS_ARG, CMDLINE_OPTION_gdb_port},
  	{"append",    HAS_ARG, CMDLINE_OPTION_kernel_cmd},
    {"uninitfb",  0,       CMDLINE_OPTION_fb_uninit},
    {"blockdev",  HAS_ARG, CMDLINE_OPTION_block_dev},
    {"trace",     HAS_ARG, CMDLINE_OPTION_trace_name},

    {NULL},
};

void parse_cmdline (int argc, char **argv, init_struct *is)
{
    int                 optind;
    const char          *r, *optarg;

    optind = 1;
    for (; ;)
    {
        if (optind >= argc)
            break;
        r = argv[optind];

        const cmdline_option *popt;

        optind++;
        /* Treat --foo the same as -foo.  */
        if (r[1] == '-')
            r++;
        popt = cmdline_options;
        for (;;)
        {
            if (!popt->name)
            {
                fprintf (stderr, "%s: invalid option -- '%s'\n", argv[0], r);
                exit (1);
            }
            if (!strcmp (popt->name, r + 1))
                break;
            popt++;
        }
        if (popt->flags & HAS_ARG)
        {
            if (optind >= argc)
            {
                fprintf (stderr, "%s: option '%s' requires an argument\n",
                        argv[0], r);
                exit (1);
            }
            optarg = argv[optind++];
        }
        else
            optarg = NULL;

        switch (popt->index)
        {
        case CMDLINE_OPTION_ncpu:
            is->no_cpus = atoi (optarg);
            break;
        case CMDLINE_OPTION_cpu_family:
            is->cpu_family = optarg;
            break;
        case CMDLINE_OPTION_cpu:
            is->cpu_model = optarg;
            break;
        case CMDLINE_OPTION_ram:
            is->ramsize = atoi (optarg) * 1024 * 1024;
            break;
        case CMDLINE_OPTION_sram:
            is->sramsize = atoi (optarg) * 1024 * 1024;
            break;
        case CMDLINE_OPTION_kernel:
            is->kernel_filename = optarg;
            break;
        case CMDLINE_OPTION_initrd:
            is->initrd_filename = optarg;
        case CMDLINE_OPTION_gdb_port:
            is->gdb_port = atoi (optarg);
            break;
		    case CMDLINE_OPTION_kernel_cmd:
			      is->kernel_cmdline = optarg;
			      break;
        case CMDLINE_OPTION_fb_uninit:
            is->fb_uninit = 1;
            break;
        case CMDLINE_OPTION_block_dev:
            is->block_device = optarg;
            break;
        case CMDLINE_OPTION_trace_name:
            is->trace_filename = optarg;
        }
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
