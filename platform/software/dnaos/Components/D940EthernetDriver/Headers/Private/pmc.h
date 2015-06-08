#ifndef SYSTEM_D940_PMC_H
#define SYSTEM_D940_PMC_H
#include <stdint.h>
#include "common.h"

#define D940_PMC_ID 1
#define D940_PMC_ADDR 0xFFFFFC00

typedef volatile union
{
  struct
  {
    uint32_t moscen:1;      /* Main Oscillator Enable */
    uint32_t oscbypass:1;   /* Oscillator Bypass */
    uint32_t __undef0:6;
    uint32_t oscount:8;     /* Main Oscillator Start-up Time */
    uint32_t __padding:16;
  } bits;
  uint32_t raw;
} mor_reg_t;

typedef volatile union
{
  struct
  {
    uint32_t mainf:16;      /* Main Clock Frequency */
    uint32_t mainrdy:1;     /* Main Clock Ready */
    uint32_t __padding:15;
  } bits;
  uint32_t raw;
} mcfr_reg_t;

typedef volatile union
{
  struct
  {
    uint32_t diva:8;        /* Divider A */
    uint32_t pllacount:6;   /* PLL A Counter */
    uint32_t outa:2;        /* PLL A Clock Frequency Range */
    uint32_t mula:11;       /* PLL A Multiplier */
    uint32_t __undef0:2;
    uint32_t one:1;         /* Always be set to 1 */
    uint32_t __padding:2;
  } bits;
  uint32_t raw;
} pllar_reg_t;

typedef volatile union
{
  struct
  {
    uint32_t divb:8;        /* Divider B */
    uint32_t pllbcount:6;   /* PLL B Counter */
    uint32_t __undef0:2;  
    uint32_t mula:11;       /* PLL B Multiplier */
    uint32_t __undef1:1;
    uint32_t usbdiv:2;      /* Divider for USB Clokc */
    uint32_t __padding:2;
  } bits;
  uint32_t raw;
} pllbr_reg_t;

typedef volatile union
{
  struct
  {
    uint32_t moscs1:1;      /* MOSCS Flag Status */
    uint32_t locka:1;       /* PLL A Lock Status */
    uint32_t lockb:1;       /* PLL B Lock Status */
    uint32_t mckrdy:1;      /* Master Clock Status */
    uint32_t __undef0:4;
    uint32_t pckrdy0:1;     /* Programmable Clock Ready Status */
    uint32_t pckrdy1:1;     /* Programmable Clock Ready Status */
    uint32_t pckrdy2:1;     /* Programmable Clock Ready Status */
    uint32_t pckrdy3:1;     /* Programmable Clock Ready Status */
    uint32_t pckrdy4:1;     /* Programmable Clock Ready Status */
    uint32_t __padding:19;
  } bits;
  uint32_t raw;
} sr_reg_t;

typedef volatile union
{
  struct
  {
    enum css_enum
    {
      slow = 0,
      main = 1,
      plla = 2,
      pllb = 3
    } css:2;                /* Master Clock Selection */

    enum pres_enum
    {
      pres_div1 = 0,
      pres_div2 = 1,
      pres_div4 = 2,
      pres_div8 = 3,
      pres_div16 = 4,
      pres_div32 = 5,
      pres_div64 = 6
    } pres:3;               /* Master Clock Selection */
    uint32_t __undef0:3;
    enum mdiv_enum
    {
      mdiv_div1 = 0,
      mdiv_div2 = 1,
      mdiv_div4 = 2
    } mdiv:2;               /* Master Clock Division */
    uint32_t __padding:22;
  } bits;
  uint32_t raw;
} mckr_reg_t;


typedef volatile struct
{
  volatile uint32_t     scer;     /* System Clock Enable Register */
  volatile uint32_t     scdr;     /* System Clock Disable Register */
  volatile uint32_t     scsr;     /* System Clock Status Register */
  volatile uint32_t     __reserved0;
  io_reg_t              pcer;     /* Peripheral Clock Enable Register */
  io_reg_t              pcdr;     /* Peripheral Clock Disable Register */
  io_reg_t              pcsr;     /* Peripheral Clock Status Register */
  volatile uint32_t     __reserved1;
  mor_reg_t             mor;      /* Main Oscillator Register */
  mcfr_reg_t            mcfr;     /* Main Clock Frequency Register */
  pllar_reg_t           pllar;    /* PLL A Register */
  pllbr_reg_t           pllbr;    /* PLL B Register */
  mckr_reg_t            mckr;     /* Master Clock Register */
  volatile uint32_t     __undef0;
  volatile uint32_t     __reserved2;
  volatile uint32_t     __reserved3;
  volatile uint32_t     pck0;     /* Programmable Clock 0 Register */
  volatile uint32_t     pck1;     /* Programmable Clock 1 Register */
  volatile uint32_t     pck2;     /* Programmable Clock 2 Register */
  volatile uint32_t     pck3;     /* Programmable Clock 3 Register */
  volatile uint32_t     pck4;     /* Programmable Clock 4 Register */
  volatile uint32_t     pck5;     /* Programmable Clock 5 Register */
  volatile uint32_t     pck6;     /* Programmable Clock 6 Register */
  volatile uint32_t     pck7;     /* Programmable Clock 7 Register */
  volatile uint32_t     ier;      /* Interrupt Enable Register */
  volatile uint32_t     idr;      /* Interrupt Disable Resister */
  sr_reg_t              sr;       /* Status Register */
  volatile uint32_t     imr;      /* Interrupt Mask Register */
  volatile uint32_t     __reserved4;
  volatile uint32_t     __reserved5;
  volatile uint32_t     __reserved6;
  volatile uint32_t     __reserved7;
  volatile uint32_t     pllicpr;  /* Charge Pump Current Register */
} pmc_regs_t;

#endif
