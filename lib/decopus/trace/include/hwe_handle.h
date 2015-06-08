
#ifndef HEV_HANDLE_H
#define HEV_HANDLE_H

#include "events/hwe_tools.h"

void handle_start(const char *tracename, int nopt, char * const opt[]);

hwe_cont * handle_alloc();

void handle_event(hwe_cont *hwe);

void handle_stop();

#endif

