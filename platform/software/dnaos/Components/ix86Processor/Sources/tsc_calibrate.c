#include <Processor/Processor.h>
#include <Processor/apic_regs.h>

/*
 * TODO: Remove this platform specific Header
 *       Move to Platform/Platform.h
 */
#include <PCPlatformDriver/Driver.h>


#define PIT_COUNTER_0       0x40
#define PIT_COUNTER_1       0x41
#define PIT_COUNTER_2       0x42
#define PIT_CONTROL         0x43
#define PIT_FQ              0x1234DD

#define CALIBRATE_MS       50
#define CALIBRATE_LATCH	   (PIT_FQ / (1000 / CALIBRATE_MS))

uint64_t                    cpu_cycles_per_ms __attribute__((__section__(".data")));
uint64_t                    cpu_bus_cycles_per_ms __attribute__((__section__(".data")));

void blocking_usleep (int us)
{
	uint64_t        tsc_end;
	
	tsc_end = get_cycles () + (us * cpu_cycles_per_ms) / 1000;
	
	while (get_cycles () < tsc_end) ;
}

void tsc_calibrate ()
{
	uint64_t        tsc_start, tsc_delta;
	uint32_t        local_timer_end;
    uint8_t         old61;

	/* Set the Gate high, disable speaker */
	old61 = inb (0x61);
	outb ((old61 & ~0x02) | 0x01, 0x61);

	// PIT Counter 2, mode 0 (interrupt on terminal count), binary count
	outb (0xb0, PIT_CONTROL);
	outb (CALIBRATE_LATCH & 0xff, PIT_COUNTER_2);
	outb (CALIBRATE_LATCH >> 8, PIT_COUNTER_2);

	tsc_start = get_cycles ();
	local_apic_mem[LAPIC_INITIAL_COUNTER >> 2] = 0xFFFFFFFF;
	while ((inb (0x61) & 0x20) == 0)
	    ;
    local_timer_end = local_apic_mem[LAPIC_CURRENT_COUNTER >> 2];
	tsc_delta = get_cycles () - tsc_start;

    local_apic_mem[LAPIC_INITIAL_COUNTER >> 2] = 0;
    outb (old61, 0x61);

    cpu_cycles_per_ms = tsc_delta / CALIBRATE_MS;
    cpu_bus_cycles_per_ms = (( 0xFFFFFFFF - local_timer_end)) / CALIBRATE_MS;

    tty_print_info ("CPU frequency=%lldMHz, CPU Bus frequency=%lldMHz\n",
        cpu_cycles_per_ms / 1000, cpu_bus_cycles_per_ms / 1000);
}

