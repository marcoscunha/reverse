#ifndef _SAVE_RESTORE_ENV_H_
#define _SAVE_RESTORE_ENV_H_

#define SAVE_ENV_BEFORE_CONSUME_SYSTEMC() \
    do {\
        qemu_instance   *_save_crt_qemu_instance = crt_qemu_instance; \
        CPUState        *_save_cpu_single_env = cpu_single_env; \
        crt_qemu_instance = NULL; \
        cpu_single_env = NULL

#define RESTORE_ENV_AFTER_CONSUME_SYSTEMC() \
        crt_qemu_instance = _save_crt_qemu_instance; \
        cpu_single_env = _save_cpu_single_env; \
    } while (0)

#ifdef IMPLEMENT_LATE_CACHES
#define CACHE_LATE_SYNC_MISSES() \
do { \
    unsigned long   ns_misses = g_crt_ns_misses;\
    if (ns_misses) { \
        g_crt_ns_misses = 0; \
        _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_ns (ns_misses); \
    } \
} while (0)
#else
#define CACHE_LATE_SYNC_MISSES()
#endif

#endif //_SAVE_RESTORE_ENV_H_
