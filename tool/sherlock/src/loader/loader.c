#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HAS_ARG 0x0001

enum{
        CMDLINE_OPTION_plugin,
};

typedef struct
{
    const char      *name;
    int             flags;
    int             index;
} cmdline_option;

const cmdline_option cmdline_options[] =
{
        {"pg",    HAS_ARG, CMDLINE_OPTION_plugin},
        {NULL},
};

int main(int argc, char** argv, char** envp)
{
    int j, i;
    char** new_env;

    int        optind = 1;
    const char *r; 
    const char *optarg;

    char *sexec = NULL;

    for (j = 0; envp[j]; j++);
        new_env = malloc((j+2) * sizeof(char*));

    if (new_env == NULL)
        printf("malloc of new_env failed.\n");

    for (i = 0; i < j; i++)
        new_env[i] = envp[i];
    // if new environment variables are desired
    // new_env[i++] = new_line;
    new_env[i++] = NULL;

    for(; sexec == NULL ;) {
        
        if(optind >= argc) break;

        r = argv[optind];

        const cmdline_option *popt;
        optind++;
       
        /* Treat --foo the same as -foo.  */
        if(r[1] == '-')
            r++;

        popt = cmdline_options;

        for(;sexec==NULL;){
            if(!popt->name){
                fprintf(stderr, "%s: invalid option -- '%s'\n", "sherlock", r);
                exit(1);
            }
            if(!strcmp(popt->name, r+1))
                break;
            popt++;
        }
        if(popt->flags & HAS_ARG){
            if(optind >= argc){
                fprintf(stderr, "%s: option '%s' requires an argument\n","sherlock", r);
                exit(1);
            }
            optarg = argv[optind++];
        }else
            optarg = NULL;

        switch(popt->index){
        case CMDLINE_OPTION_plugin:
       
            sexec = (char*)malloc(strlen("bin/")+strlen(optarg)+1);
            sprintf(sexec, "bin/%s",optarg);
            printf("Executing sherlock + %s\n", optarg );

            break;
        }
    }

    if(execve(sexec,&argv[--optind],new_env)){ // Execute the plugin - delorean - reverse debug
        free(sexec);
        perror("Error");
        exit(1);
    }



}


