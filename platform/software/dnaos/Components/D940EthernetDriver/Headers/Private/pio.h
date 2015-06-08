#ifndef SYSTEM_D940_PIO_H
#define SYSTEM_D940_PIO_H
#include <stdint.h>
#include "common.h"

#define D940_PIO_A_ID 2
#define D940_PIO_B_ID 3
#define D940_PIO_C_ID 4

#define D940_PIO_A_ADDR  0xFFFFF400
#define D940_PIO_B_ADDR  0xFFFFF600
#define D940_PIO_C_ADDR  0xFFFFF800

typedef volatile struct
{
  io_reg_t         per;          /* PIO Enable Register */
  io_reg_t         pdr;          /* PIO Disable Register */
  io_reg_t         psr;          /* PIO Status Register */
  io_reg_t         __reserved0;
  io_reg_t         oer;          /* Output Enable Register */
  io_reg_t         odr;          /* Output Disable Register */
  io_reg_t         osr;          /* Output Status Register */
  io_reg_t         __reserved1;
  io_reg_t         ifer;         /* Glitch Input Filter Enable Register */
  io_reg_t         ifdr;         /* Glitch Input Filter Disable Register */
  io_reg_t         ifsr;         /* Glitch Input Filter Status Register */
  io_reg_t         __reserved2;
  io_reg_t         sodr;         /* Set Output Data Register */
  io_reg_t         codr;         /* Clear Output Data Register */
  io_reg_t         odsr;         /* Output Data Status Register */
  io_reg_t         pdsr;         /* Pin Data Status Register */
  io_reg_t         ier;          /* Interrupt Enable Register */
  io_reg_t         idr;          /* Interrupt Disable Register */
  io_reg_t         imr;          /* Interrupt Mask Register */
  io_reg_t         isr;          /* Interrupt Status Register */
  io_reg_t         mder;         /* Multi-driver Enable Register */
  io_reg_t         mddr;         /* Multi-driver Disable Register */
  io_reg_t         mdsr;         /* Multi-driver Status Register */
  io_reg_t         __reserved3;
  io_reg_t         pudr;         /* Pull-up Disable Register */
  io_reg_t         puer;         /* Pull-up Enable Register */
  io_reg_t         pusr;         /* Pad Pull-up Status Register */
  io_reg_t         __reserved4;
  io_reg_t         asr;          /* Peripheral A Select Register */
  io_reg_t         bsr;          /* Peripheral B Select Register */
  io_reg_t         absr;         /* AB Status Register */
  io_reg_t         __reserved5[9];
  io_reg_t         ower;         /* Output Write Enable */
  io_reg_t         owdr;         /* Output Write Disable */
  io_reg_t         owsr;         /* Output Write Status Register */
  io_reg_t         __reserved6;
} pio_regs_t;

#endif
