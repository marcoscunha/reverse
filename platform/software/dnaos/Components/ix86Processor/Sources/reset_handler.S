GDT_ENTRY_BOOT_CS               = 2
__BOOT_CS                       = 16

GDT_ENTRY_BOOT_DS               = 3
__BOOT_DS                       = 24

IDT_ENTRIES                     = 256

LOCAL_APIC_BASE                 = 0xFEE00000
LAPIC_CPU_ID                    = 0x020
LAPIC_EOI                       = 0x0B0
LAPIC_SPURIOUS                  = 0x0F0
LAPIC_ICR_LOW                   = 0x300
LAPIC_LVT_ERR                   = 0x370
LOCAL_TIMER_LVT                 = 0x320
LOCAL_TIMER_INITIAL_COUNTER     = 0x380
LOCAL_TIMER_CURRENT_COUNTER     = 0x390
LOCAL_COUNTER_DIVIDER           = 0x3E0
LOCAL_TIMER_VECTOR              = 0XEF
LAPIC_ERROR_VECTOR              = 0xEE
LAPIC_IPI_VECTOR                = 0xED

    .code32
    .section ".reset","ax"

    .globl reset_handler;
reset_handler:

    lgdt    boot_gdt_descr
    movl    $__BOOT_DS,%eax
    movl    %eax, %ds
    movl    %eax, %es
    movl    %eax, %fs
    movl    %eax, %gs

    ljmp    $__BOOT_CS, $3f    /* Clear prefetch and normalize %eip */

3:
    cmp     $1, first_cpu
    jne     4f
    movl    $0, first_cpu
    movl    CPU_SYS_STACK_ADDR, %esp
    call    idt_setup
    call    local_apic_setup

    call    tsc_calibrate
    sti
    jmp     5f

4:
    lidt    boot_idt_descr      # cpu != 0
    call    local_apic_setup
    
    # Compute the basic stack addresses
    call    cpu_mp_id
    mov     %eax, %ebp
    mov     $0x400, %ecx
    mul     %ecx
    movl    CPU_SYS_STACK_ADDR, %ecx
    subl    %eax, %ecx
    movl    %ecx, %esp
    
    sti

5:
    # Call cpu_start
    movl    $cpu_start, %eax
    jmp     * %eax
    nop

idt_setup:
    lea     ignore_int, %edx
    movl    $__BOOT_CS << 16, %eax
    movw    %dx, %ax        /* selector = 0x0010 = cs */
    movw    $0x8E00, %dx    /* interrupt gate - dpl=0, present */

    lea     boot_idt_table, %edi
    mov     $256, %ecx
rp_sidt:
    movl    %eax, (%edi)
    movl    %edx, 4(%edi)
    addl    $8, %edi
    dec     %ecx
    jne     rp_sidt

    .macro  set_early_handler handler, trapno
            lea     \handler, %edx
            movl    $__BOOT_CS << 16, %eax
            movw    %dx, %ax
            movw    $0x8E00, %dx    /* interrupt gate - dpl=0, present */
            lea     boot_idt_table, %edi
            movl    %eax,8 * \trapno (%edi)
            movl    %edx, 8 * \trapno + 4(%edi)
    .endm

    set_early_handler handler = divide_err, trapno = 0
    set_early_handler handler = illegal_opcode, trapno = 6
    set_early_handler handler = protection_fault, trapno = 13
    set_early_handler handler = timer_int, trapno = LOCAL_TIMER_VECTOR
    set_early_handler handler = ipi_int, trapno = LAPIC_IPI_VECTOR
    set_early_handler handler = lapic_error_int, trapno = LAPIC_ERROR_VECTOR

    lidt    boot_idt_descr

    ret /*idt_setup*/

lapic_error_int:
    pusha
    call    cpu_mp_id
    push    %eax
    push    $msg_lapic_error
    call    tty_print_info
    add     $8, %esp
    popa

    iret /* lapic_error_int */

local_apic_setup:
	movl    $1, %eax            # see if this cpu has rdmsr/wrmsr instructions
	cpuid
	bt      $9, %edx            # see if this chip has an local apic
	jnc     no_lapic            # if not, can't use apic
	bt      $5, %edx            # check for rdmsr instruction capability
	jnc     apic_default
	movl    $0x1B, %ecx         # ok, read the apic physical base address
	rdmsr                       # ... into eax
	orl     $0x800, %eax		# set the global enable bit
	andl    $0xFFF, %eax
	orl     $LOCAL_APIC_BASE, %eax
	wrmsr
apic_default:
    # vector EF, one shot
    movl    $LOCAL_TIMER_VECTOR, (LOCAL_APIC_BASE + LOCAL_TIMER_LVT)
    # divide by 1
    movl    $0x0B,  (LOCAL_APIC_BASE + LOCAL_COUNTER_DIVIDER)
    movl    $0x00, (LOCAL_APIC_BASE + LOCAL_TIMER_INITIAL_COUNTER)
    movl    (LOCAL_APIC_BASE + LAPIC_SPURIOUS), %eax    #enable IPI
    orl     $0x100, %eax
    movl    %eax, (LOCAL_APIC_BASE + LAPIC_SPURIOUS)

    ret                         # local_apic_setup
no_lapic:
    pushl   $msg_no_lapic
    call    tty_print_info
    jmp     _hell_pit

divide_err:
    xor     %edx, %edx
    pushl   $0    /* fake errcode */
    jmp     fatal_fault

illegal_opcode:
    movl    $6, %edx
    pushl   $0    /* fake errcode */
    jmp     fatal_fault

protection_fault:
    movl    $13, %edx
    jmp     fatal_fault

fatal_fault:
    call    cpu_mp_id
    push    %eax
    push    %edx
    push    $msg_fatal_fault
    call    tty_print_info
    add     $0x0C, %esp
    jmp     _hell_pit

    /* default interrupt handler */
    .align  4
ignore_int:
    pusha
    call    cpu_mp_id
    push    %eax
    push    $msg_ignored_int
    call    tty_print_info
    add     $8, %esp
    popa

    iret

    .align 4
timer_int:
    pusha

    # call    cpu_mp_id
    # push    %eax
    # push    $msg_timer_int
    # call    tty_print_info
    # add     $8, %esp
    
    pushl   $1
    movl    isr_timer, %eax
    call    * %eax
    add     $4, %esp

    #EOI
    movl    $0, (LAPIC_EOI + LOCAL_APIC_BASE)

    popa

    iret

    .align 4
ipi_int:
    pusha

    # call    cpu_mp_id
    # push    %eax
    # push    $msg_timer_int
    # call    tty_print_info
    # add     $8, %esp
    
    pushl   $1
    movl    isr_ipi, %eax
    call    * %eax
    add     $4, %esp

    #EOI
    movl    $0, (LAPIC_EOI + LOCAL_APIC_BASE)

    popa

    iret

_hell_pit:
    call    cpu_mp_id
    push    %eax
    pushl   $msg_hell_pit
    call    tty_print_info
    add     $8, %esp
    hlt
    jmp     _hell_pit

.data
    .align  4
first_cpu: .long 1
    .globl  boot_gdt_descr
boot_gdt_descr:
    .word   __BOOT_DS + 7
    .long   boot_gdt

    .align  16
    .globl  boot_gdt
boot_gdt:
    .fill   GDT_ENTRY_BOOT_CS, 8, 0
    .quad   0x00cf9b000000ffff // flat 32-bit code segment
    .quad   0x00cf93000000ffff // flat 32-bit data segment

    .align  4
    .globl  boot_idt_descr
boot_idt_descr:
    .word   IDT_ENTRIES * 8 - 1   # idt contains 256 entries
    .long   boot_idt_table
    
    .align  16
    .globl  boot_idt_table
boot_idt_table:
    .fill   IDT_ENTRIES, 8, 0

    # messages
msg_ignored_int: .string "An ignored interrupt occured on cpu %ld.\n"
msg_fatal_fault: .string "Fatal interrupt %ld (code 0x%lx) occured on cpu %ld.\n"
msg_hell_pit: .string "Hell pit reached by cpu %ld.\n"
msg_no_lapic: .string "No local local APIC found on this computer.\n"
msg_timer_int: .string "Timer interrupt occured on cpu %ld.\n"
msg_lapic_error: .string "Local APIC error interrupt occured on cpu %ld.\n"

