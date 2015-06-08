static inline void flush_write_buffers (void)
{
    //linux definition, file <linux_dir>/arch/x86/include/asm/io_32.h
    #if defined (CONFIG_X86_OOSTORE) || defined (CONFIG_X86_PPRO_FENCE)
	asm volatile("lock; addl $0,0(%%esp)": : :"memory");
    #endif
}

void cpu_cache_sync (void)
{                             
	flush_write_buffers ();
}


