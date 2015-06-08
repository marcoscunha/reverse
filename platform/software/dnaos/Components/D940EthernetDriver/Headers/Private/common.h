#ifndef SYSTEM_D940_COMMON_H
#define SYSTEM_D940_COMMON_H
typedef volatile union
{
  struct
  {
    uint32_t p0:1;      /* Corresponding I/O line */
    uint32_t p1:1;      /* Corresponding I/O line */
    uint32_t p2:1;      /* Corresponding I/O line */
    uint32_t p3:1;      /* Corresponding I/O line */
    uint32_t p4:1;      /* Corresponding I/O line */
    uint32_t p5:1;      /* Corresponding I/O line */
    uint32_t p6:1;      /* Corresponding I/O line */
    uint32_t p7:1;      /* Corresponding I/O line */
    uint32_t p8:1;      /* Corresponding I/O line */
    uint32_t p9:1;      /* Corresponding I/O line */
    uint32_t p10:1;     /* Corresponding I/O line */
    uint32_t p11:1;     /* Corresponding I/O line */
    uint32_t p12:1;     /* Corresponding I/O line */
    uint32_t p13:1;     /* Corresponding I/O line */
    uint32_t p14:1;     /* Corresponding I/O line */
    uint32_t p15:1;     /* Corresponding I/O line */
    uint32_t p16:1;     /* Corresponding I/O line */
    uint32_t p17:1;     /* Corresponding I/O line */
    uint32_t p18:1;     /* Corresponding I/O line */
    uint32_t p19:1;     /* Corresponding I/O line */
    uint32_t p20:1;     /* Corresponding I/O line */
    uint32_t p21:1;     /* Corresponding I/O line */
    uint32_t p22:1;     /* Corresponding I/O line */
    uint32_t p23:1;     /* Corresponding I/O line */
    uint32_t p24:1;     /* Corresponding I/O line */
    uint32_t p25:1;     /* Corresponding I/O line */
    uint32_t p26:1;     /* Corresponding I/O line */
    uint32_t p27:1;     /* Corresponding I/O line */
    uint32_t p28:1;     /* Corresponding I/O line */
    uint32_t p29:1;     /* Corresponding I/O line */
    uint32_t p30:1;     /* Corresponding I/O line */
    uint32_t p31:1;     /* Corresponding I/O line */
  } bits;
  uint32_t raw;
} io_reg_t;

#endif
