#ifndef _HWETRACE_CPU_H_
#define _HWETRACE_CPU_H_

#include "hwetrace_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TR_NO_EXCL      0x00
#define TR_EXCL_LOAD    0x01
#define TR_EXCL_STORE   0x02

/*
 * Setters functions for processors traces.
 *
 * defines:
 * HWE_CPU_INST_NODATE -> enable date in instruction
 * HWE_CPU_DREG_NOID   -> enable modified register (id only)
 * HWE_CPU_DREG_NODATA -> enable data of modified register (need previous)
 * HWE_CPU_IMEM_NODATE -> enable dates in instruction memory request
 * HWE_CPU_IMEM_NODATA -> enable data in instruction memory request
 * HWE_CPU_DMEM_NODATE -> enable dates in data memory request
 * HWE_CPU_DMEM_NODATA -> enable data in data memory request
 * HWE_CPU_IMEM_NOACK  -> request are not followed by ack
 * HWE_CPU_DMEM_NOACK  -> request are not followed by ack
 */

#if HWETRACE_IMEM == 1
#define HWE_CPU_IMEM_NO_ACK
#endif

#if HWETRACE_DMEM == 1
#define HWE_CPU_DMEM_NO_ACK
#endif

#ifdef HWETRACE_NODATA
#define HWE_CPU_IMEM_NODATA
#define HWE_CPU_DMEM_NODATA
#define HWE_CPU_DREG_NODATA
#endif
#ifdef HWETRACE_NODATE
#define HWE_CPU_IMEM_NODATE
#define HWE_CPU_DMEM_NODATE
#define HWE_CPU_INST_NODATE
#endif
#ifdef HWETRACE_NOREG
#define HWE_CPU_DREG_NOID
#endif

#ifdef HWE_CPU_DREG_NOID
#define HWE_CPU_DREG_NODATA
#endif

/***********************
 * instruction setters *
 ***********************/
static inline void HWE_CPU_inst_init (hwe_cont *cont,
      uint32_t pc,
      unsigned int width)
{
#ifdef HWE_CPU_INST_NODATE
   HWE_HEAD_init(cont, HWE_INST32, 0, 0);
#else 
   HWE_HEAD_init(cont, HWE_INST32, 0, 1);
#endif
#ifdef HWE_CPU_BODY_INSTR
   cont->inst.body.instr = 0x00;
#endif
   cont->inst.body.pc    = pc;
   cont->inst.body.width = width - 1;
   cont->inst.body.abort = 0;
   cont->inst.body.jump  = 0;
   cont->inst.body.str   = 0;
#ifdef HWE_CPU_DREG_NODATA
   cont->inst.body.regdata = 0;
#else
   cont->inst.body.regdata = 1;
#endif
   cont->inst.body.nregs = 0;
#ifdef HWE_CPU_ARM
    cont->inst.body.exec    = 1;
    cont->inst.body.cycles  = 0;
    cont->inst.body.unalign = 0;
    cont->inst.body.excl    = 0;
    cont->inst.body.imem    = 1;
    cont->inst.body.dmem    = 0;
#endif
    cont->inst.ack_commit   = 0;
}

static inline uint32_t HWE_CPU_inst_get_pc(
      hwe_cont *cont)
{
   return cont->inst.body.pc;
}

static inline void HWE_CPU_inst_set_jump(
      hwe_cont *cont, 
      uint32_t jpc)
{
   cont->inst.body.jump = 1;
   cont->inst.jump_pc = jpc;
}

static inline int HWE_CPU_inst_is_jump(
     hwe_cont *cont)
{
   return  cont->inst.body.jump;
}

static inline void HWE_CPU_inst_set_instr(
    hwe_cont *cont,
    uint32_t instr)
{
    cont->inst.body.instr = instr;
}

static inline void HWE_CPU_inst_set_cycles(
    hwe_cont *cont,
    uint32_t cycles)
{
    cont->inst.body.cycles = cycles;
}

static inline void HWE_CPU_inst_inc_cycles(
    hwe_cont *cont,
    uint32_t cycles)
{
    cont->inst.body.cycles += cycles;
}

static inline unsigned HWE_CPU_inst_get_cycles(
    hwe_cont *cont)
{
    return cont->inst.body.cycles;
}

static inline void HWE_CPU_inst_clr_exec(
    hwe_cont *cont)
{
    cont->inst.body.exec = 0;
}

static inline void HWE_CPU_inst_set_dmem(
    hwe_cont *cont,
    int i)
{
    cont->inst.body.dmem = i;
}

static inline void HWE_CPU_inst_inc_dmem(
    hwe_cont *cont,
    int i)
{
    cont->inst.body.dmem += i;
}

static inline unsigned HWE_CPU_inst_get_dmem(
    hwe_cont *cont)
{
    return cont->inst.body.dmem;
}

static inline void HWE_CPU_inst_set_imem(
    hwe_cont *cont,
    int i)
{
    cont->inst.body.imem = i;
}


static inline void HWE_CPU_inst_inc_imem(
    hwe_cont *cont,
    int i)
{
    cont->inst.body.imem += i;
}

static inline unsigned HWE_CPU_inst_get_imem(
    hwe_cont *cont)
{
    return cont->inst.body.imem;
}

static inline void HWE_CPU_inst_set_unalign(
    hwe_cont *cont)
{
    cont->inst.body.unalign = 1;
}

static inline unsigned HWE_CPU_inst_get_unalig(
    hwe_cont *cont)
{
    return cont->inst.body.unalign;
}

static inline void HWE_CPU_inst_set_excl(
    hwe_cont *cont,
    unsigned exclusive)
{
    cont->inst.body.excl = exclusive;
}

static inline unsigned HWE_CPU_inst_get_excl(
    hwe_cont *cont)
{
    return cont->inst.body.excl;
}

static inline void HWE_CPU_inst_set_date(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifdef HWE_CPU_INST_NODATE
   ;
#else
   cont->common.dates[0] = date;
#endif
}

#ifdef HWE_CPU_BODY_INSTR
static inline void HWE_CPU_inst_set_instr(
      hwe_cont *cont __attribute__((__unused__)),
      uint32_t instr __attribute__((__unused__)))
{
   cont->body.instr = instr;

}
#endif


static inline void HWE_CPU_inst_set_nreg(
      hwe_cont *cont __attribute__((__unused__)), 
      unsigned int nregs __attribute__((__unused__)))
{
#ifdef HWE_CPU_DREG_NOID
   ;
#else
   cont->inst.body.nregs = (nregs <= HWE_INST_REG_MAX) 
      ? nregs 
      : HWE_INST_REG_MAX;
#endif
}

static inline unsigned int HWE_CPU_inst_add_reg(
      hwe_cont *cont __attribute((__unused__)))
{
#ifdef HWE_CPU_DREG_NOID
   return 0;
#else
   if (cont->inst.body.nregs < HWE_INST_REG_MAX)
      return cont->inst.body.nregs++;
   return HWE_INST_REG_MAX - 1;
#endif
}

static inline void HWE_CPU_inst_set_regid(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned int index __attribute__((__unused__)),
      uint8_t id __attribute__((__unused__)))
{
#ifdef HWE_CPU_DREG_NOID
   ;
#else
   if (index < HWE_INST_REG_MAX)
      cont->inst.reg_id[index] = id;
#endif
}

static inline void HWE_CPU_inst_set_regdst(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned int index __attribute__((__unused__)),
      bool isdst __attribute__((__unused__)))
{
#ifdef HWE_CPU_DREG_NOID
   ;
#else
   if (index < HWE_INST_REG_MAX) {
		if (isdst)
			cont->inst.body.regdst |= ((uint32_t)1U << index);
		else
			cont->inst.body.regdst &= ~((uint32_t)1U << index);
	}
#endif
}

static inline void HWE_CPU_inst_set_regdata(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned int index __attribute__((__unused__)),
      uint32_t data __attribute__((__unused__)))
{
#ifdef HWE_CPU_DREG_NODATA
   ;
#else
   if (index < HWE_INST_REG_MAX)
      cont->inst.reg_data[index] = data;
#endif
}

static inline void HWE_CPU_inst_set_reg(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned int index __attribute__((__unused__)),
      bool    isdst __attribute__((__unused__)),
      uint8_t id __attribute__((__unused__)),
      uint32_t data __attribute__((__unused__)))
{
   HWE_CPU_inst_set_regid(cont, index, id);
   HWE_CPU_inst_set_regdata(cont, index, data);
   HWE_CPU_inst_set_regdst(cont, index, isdst);
}

static inline unsigned int HWE_CPU_inst_get_nreg(
      hwe_cont *cont __attribute((__unused__)))
{
#ifdef HWE_CPU_DREG_NOID
   return 0;
#else
   return cont->inst.body.nregs;
#endif
}

static inline uint32_t HWE_CPU_inst_get_regid(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned int index __attribute__((__unused__)))
{
#ifdef HWE_CPU_DREG_NOID
   return 0;
#else
   if (index < HWE_INST_REG_MAX)
      return  cont->inst.reg_id[index];
   else
      return HWE_INST_REG_MAX + 1;
#endif
}

static inline bool HWE_CPU_inst_has_reg(
      hwe_cont *cont __attribute__((__unused__)))
{
    return HWE_CPU_inst_get_nreg(cont) ? true : false;
}

/**************************************
 * instruction memory request setters *
 **************************************/
static inline void HWE_CPU_imem_init(
      hwe_cont *cont,
      unsigned int nrefs,
      uint32_t addr,
      unsigned int width)
{
   HWE_MEM32_init(cont, nrefs, true, HWE_MEM_LOAD, addr, width);
#ifdef HWE_CPU_IMEM_NOACK
   cont->common.head.expected = 0;
#endif
#ifdef HWE_CPU_IMEM_NODATE
   cont->common.head.ndates = 0;
#endif
#ifdef HWE_CPU_IMEM_NODATA
   cont->mem.body.mem32.usedata = 0;
#endif
}

static inline void HWE_CPU_imem_begdate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifdef HWE_CPU_IMEM_NODATE
   ;
#else
   HWE_HEAD_set_date(cont, 0, date);
#endif
}

static inline void HWE_CPU_imem_enddate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifdef HWE_CPU_IMEM_NODAT1E
   ;
#else
   HWE_HEAD_set_date(cont, 1, date);
#endif
}

static inline void HWE_CPU_imem_set_byte(
      hwe_cont *cont __attribute__((__unused__)), 
      uint8_t index __attribute__((__unused__)),
      uint8_t byte __attribute__((__unused__)))
{
#ifdef HWE_CPU_IMEM_NODATA
   ;
#else
   HWE_MEM32_set_byte(cont, index, byte);
#endif
}

static inline void HWE_CPU_imem_set_word(
      hwe_cont *cont __attribute__((__unused__)), 
      uint8_t index __attribute__((__unused__)),
      uint32_t word __attribute__((__unused__)))
{
#ifdef HWE_CPU_IMEM_NODATA
   ;
#else
   HWE_MEM32_set_word(cont, index, word);
#endif
}

static inline void HWE_CPU_imem_set_data(
      hwe_cont *cont __attribute__((__unused__)), 
      void *data __attribute__((__unused__)))
{
#ifdef HWE_CPU_IMEM_NODATA
  ; 
#else
   HWE_MEM32_set_data(cont, data);
#endif
}

/*******************************
 * data memory request setters *
 *******************************/
static inline void HWE_CPU_dmem_init(
      hwe_cont *cont,
      unsigned int nrefs,
      hwe_mem_t access,
      uint32_t addr,
      unsigned int width)
{
   HWE_MEM32_init(cont, nrefs, false, access, addr, width); 
#ifdef HWE_CPU_IMEM_NOACK
   HWE_HEAD_set_expected(cont, 0);
#endif
#ifdef HWE_CPU_DMEM_NODATE
   cont->common.head.ndates = 0;
#endif
#ifdef HWE_CPU_DMEM_NODATA
   cont->mem.body.mem32.usedata = 0;
#endif
}

static inline void HWE_CPU_dmem_unknown(
      hwe_cont *cont,
      unsigned int nrefs)
{
#ifdef HWE_CPU_DMEM_NODATE
   HWE_HEAD_init(cont, HWE_NULL, nrefs, 0); 
#else
   HWE_HEAD_init(cont, HWE_NULL, nrefs, 2); 
#endif
#ifdef HWE_CPU_IMEM_NOACK
   HWE_HEAD_set_expected(cont, 0);
#else
   HWE_HEAD_set_expected(cont, 1);
#endif
}

static inline void HWE_CPU_dmemgl_init(
      hwe_cont *cont,
      unsigned int nrefs,
      hwe_mem_t access)
{
   HWE_MEMGL_init(cont, nrefs, access);
#ifdef HWE_CPU_IMEM_NOACK
   HWE_HEAD_set_expected(cont, 0);
#endif
#ifdef HWE_CPU_DMEM_NODATE
   cont->common.head.ndates = 0;
#endif
} 

static inline void HWE_CPU_dmem_begdate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifdef HWE_CPU_DMEM_NODATE
   ;
#else
   HWE_HEAD_set_date(cont, 0, date);
#endif
}

static inline void HWE_CPU_dmem_enddate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifdef HWE_CPU_DMEM_NODATE
   ;
#else
   HWE_HEAD_set_date(cont, 1, date);
#endif
}

static inline void HWE_CPU_dmem_set_byte(
      hwe_cont *cont __attribute__((__unused__)), 
      uint8_t index __attribute__((__unused__)),
      uint8_t byte __attribute__((__unused__)))
{
#ifdef HWE_CPU_DMEM_NODATA
   ;
#else
   HWE_MEM32_set_byte(cont, index, byte);
#endif
}

static inline void HWE_CPU_dmem_set_word(
      hwe_cont *cont __attribute__((__unused__)), 
      uint8_t index __attribute__((__unused__)),
      uint32_t word __attribute__((__unused__)))
{
#ifdef HWE_CPU_DMEM_NODATA
   ;
#else
   HWE_MEM32_set_word(cont, index, word);
#endif
}

static inline void HWE_CPU_dmem_set_data(
      hwe_cont *cont __attribute__((__unused__)), 
      void *data __attribute__((__unused__)))
{
#ifdef HWE_CPU_DMEM_NODATA
  ; 
#else
   HWE_MEM32_set_data(cont, data);
#endif
}
/*******************************
 * I/O  request setters *
*******************************/
static inline void HWE_CPU_io_init(
      hwe_cont *cont,
      unsigned int nrefs,
      uint32_t addr,
      unsigned int width)
{
   HWE_HEAD_init(cont, HWE_CPU_IO, 0, 0);
}


static inline void HWE_CPU_mem_init(
      hwe_cont *cont,
      unsigned int nrefs,
      uint32_t addr,
      unsigned int width)
{
    HWE_HEAD_init(cont, HWE_CPU_MEM, 0, 0);

}

 




#ifdef __cplusplus
}
#endif

#endif // _HWETRACE_CPU_H_

