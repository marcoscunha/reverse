#ifndef _CMD_H_
#define _CMD_H_

typedef struct
{
    int                 gdb_port;
    char*               elf_filename;
} init_struct;



void parse_cmdline (int argc, char **argv, init_struct *is);

#endif

