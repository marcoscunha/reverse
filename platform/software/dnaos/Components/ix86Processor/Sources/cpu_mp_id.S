    .globl cpu_mp_id
cpu_mp_id:
    movl    0xFEE00020, %eax
    shrl    $0x18, %eax
    andl    $0x1F, %eax
    ret

