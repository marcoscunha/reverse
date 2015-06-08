#ifndef SILLY_APIC_H
#define SILLY_APIC_H

#define LOCAL_APIC_BASE             0xFEE00000
#define LAPIC_CPU_ID                0x20
#define LAPIC_EOI                   0xB0
#define LAPIC_SPURIOUS              0x0F0
#define LAPIC_ICR_LOW               0x300
#define LAPIC_ICR_HIGH              0x310
#define LAPIC_TIMER_LVT             0x320
#define LAPIC_LVT_ERR               0x370
#define LAPIC_INITIAL_COUNTER       0x380
#define LAPIC_CURRENT_COUNTER       0x390
#define LAPIC_DIVIDE                0x3E0
#define LAPIC_ERROR_VECTOR          0xEE
#define LAPIC_IPI_VECTOR            0xED

#endif

