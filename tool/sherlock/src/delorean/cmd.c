#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cmd.h>


#define HAS_ARG             0x0001

enum
{
    CMDLINE_OPTION_gdb_port,
};


typedef struct
{
    const char      *name;
    int             flags;
    int             index;
} cmdline_option;

const cmdline_option cmdline_options[] =
{
    {"gdb_port",  HAS_ARG, CMDLINE_OPTION_gdb_port},
    {NULL},
};

void parse_cmdline (int argc, char **argv, init_struct *is)
{
    int                 optind;
    const char          *r, *optarg;

    optind = 0;

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
                fprintf (stderr, "%s: invalid option -- '%s'\n", "sherlock", r);
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
                        "sherlock", r);
                exit (1);
            }
            optarg = argv[optind++];
        }
        else
            optarg = NULL;

        switch (popt->index){
        case CMDLINE_OPTION_gdb_port:
            is->gdb_port = atoi (optarg);
        break;
        }
    }
}

