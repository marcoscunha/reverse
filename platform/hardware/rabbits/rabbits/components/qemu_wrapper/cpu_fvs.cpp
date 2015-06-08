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

#include <string.h>
#include "cfg.h"
#include "cpu_fvs.h"

static unsigned long   arm_cpu_fv[] = {25, 50, 100, 150, 200, 250, 300, 0};

static unsigned long   arm_cortexa9_cpu_fv[] = {300, 600, 1000, 0};


int get_cpu_nb_fv_levels (const char *cpufamily, const char *cpumodel)
{
    if (!strcmp(cpufamily,"arm") && !strcmp(cpumodel,"cortex-a9"))
        return sizeof (arm_cortexa9_cpu_fv) / sizeof (arm_cortexa9_cpu_fv[0]) - 1;

    return sizeof (arm_cpu_fv) / sizeof (arm_cpu_fv[0]) - 1;
}

int get_cpu_boot_fv_level (const char *cpufamily, const char *cpumodel)
{
    if (!strcmp(cpufamily,"arm") && !strcmp(cpumodel,"cortex-a9"))    
        return sizeof (arm_cortexa9_cpu_fv) / sizeof (arm_cortexa9_cpu_fv[0]) - 2;

    return sizeof (arm_cpu_fv) / sizeof (arm_cpu_fv[0]) - 2;
}

unsigned long  *get_cpu_fvs (const char *cpufamily, const char *cpumodel)
{
    if (!strcmp(cpufamily,"arm") && !strcmp(cpumodel,"cortex-a9"))
        return arm_cortexa9_cpu_fv;

    return arm_cpu_fv;
}

#ifdef ENERGY_TRACE_ENABLED
static uint64_t        etrace_arm_class_mode_cost[] =
    {23, 83, 26, 90, 29, 97, 34, 123, 38, 143, 60, 215, 90, 320};
static periph_class_t  etrace_arm_class =
{
    /*.class_name = */"ARM_CPU",
    /*.max_energy_ms = */400,
    /*.nmodes = */2,
    /*.nfvsets = */7,
    /*.mode_cost = */etrace_arm_class_mode_cost
};

static uint64_t        etrace_arm_cortexa9_class_mode_cost[] =
    {38, 143, 60, 215, 90, 320};
static periph_class_t  etrace_arm_cortexa9_class =
{
    /*.class_name = */"ARM_CORTEX19_CPU",
    /*.max_energy_ms = */400,
    /*.nmodes = */2,
    /*.nfvsets = */3,
    /*.mode_cost = */etrace_arm_cortexa9_class_mode_cost
};

periph_class_t *get_cpu_etrace_class (const char *cpufamily, const char *cpumodel)
{
    if (!strcmp(cpufamily,"arm") && !strcmp(cpumodel,"cortex-a9"))
    return &etrace_arm_cortexa9_class;

    return &etrace_arm_class;
}
#endif

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
