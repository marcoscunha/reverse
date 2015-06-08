#ifndef _HWE_INST_H_
#define _HWE_INST_H_

#include "hwe_common.h"

/*
 * Instruction related events
 */

/**********************************************************************
 ********     HWE_INST32     ******************************************
 **********************************************************************/

/**
 *
 * Instruction on 32bits address / 32bits data architecture
 * 
 * @struct mapping:
 *    HEADER;
 *    hwe_inst32_t body;
 *    uint8_t      reg_id[body.nregs];
 *    uint32_t     reg_data[body.nregs];
 */
typedef struct hwe_inst32_t {
   uint32_t    pc;        /**< access address                                 */
   unsigned    width:4;   /**< width of the instruction - 1 in bytes          */
   unsigned    abort:1;   /**< Was instruction  aborted ?                      */
   unsigned    jump:1;    /**< tell if next instruction follow  this one or not*/
   unsigned    regdata:1; /**< tell if register's new values is present in the event*/
   unsigned    exec:1;    /**< Is it executed ?                        */
   unsigned    unalign:1;  /**< Is it an aligned access ?               */
   unsigned    cycles:8;
   unsigned    imem:4;    /**< Number of instruction cache access      */
   unsigned    dmem:4;    /**< Number of data cache access             */
   unsigned    excl:2;    /**< Is it an exclusive access (load/store) ? */
   unsigned    str:1;     /**< Did it trigger a write allocate (write-back only) ? */
   int:0;    
   unsigned    nregs;     /**< number of used registers                      */
   uint32_t    regdst;    /**< src or dst registers                          */
   uint32_t    instr;     /**< Instruction                                   */

} __attribute__((__packed__)) hwe_inst32_t;

/*
 * Exception on 32bits address / 32bits data architecture
 * 
 * mapping:
 *    HEADER;
 *    hwe_excep32_t body;
 */
typedef struct hwe_excep32_t {
   uint32_t    retpc;       // pc which will be used to return from exception
} __attribute__((__packed__)) hwe_excep32_t;

typedef struct hwe_cpu32_t {
   uint32_t   access:2; /**<  LOAD / STORE */
   uint32_t   io_mem:1; /**< MEMORY OR IO */
   uint32_t   inst:1;   /**< Access to instruction or data */
   int:0;
} __attribute__((__packed__)) hwe_cpu32_t;

#define HWE_INST_REG_MAX 18 // For ARMv7 arch

/*
 * container
 */
typedef struct hwe_inst_cont {
   hwe_head_cont common;
   hwe_inst32_t  body;
   uint32_t      jump_pc; // present if jump==1
   uint8_t       reg_id[HWE_INST_REG_MAX];   // id of modified registers
   uint32_t      reg_data[HWE_INST_REG_MAX]; // data of modified registers
   bool          ack_commit;
} hwe_inst_cont;
typedef struct hwe_excep_cont {
   hwe_head_cont common;
   hwe_excep32_t  body;
} hwe_excep_cont;
typedef struct hwe_cpu_cont{
   hwe_head_cont common;
   hwe_cpu32_t   body;
} hwe_cpu_cont;



#endif // _HWE_INST_H_

/**********************************************************************
 ********     TOOLS     ***********************************************
 **********************************************************************/

#ifdef HWE_USE_TOOLS
#ifndef _HWE_TOOLS_INST_H_
#define _HWE_TOOLS_INST_H_


/*
 * compute the size needed to store an event
 */
static inline size_t hwe_inst32_sizeof(const hwe_inst_cont *cont)
{
   size_t res = hwe_head_sizeof(&cont->common);
   res += sizeof(hwe_inst32_t) + cont->body.nregs;
   if (cont->body.jump)
      res += 4U;
   if (cont->body.regdata)
      res += 4U * cont->body.nregs;
   return res;
}
static inline size_t hwe_excep32_sizeof(const hwe_excep_cont *cont)
{
   return hwe_head_sizeof(&cont->common) + sizeof(hwe_excep32_t);
}

static inline size_t hwe_cpu32_sizeof(const hwe_cpu_cont *cont)
{
   return hwe_head_sizeof(&cont->common) + sizeof(hwe_cpu32_t);
}


/*
 * write an event to the given memory location
 * which must have enough allocated space
 * return the address following the event
 */
static inline void * hwe_inst32_write(const hwe_inst_cont *cont, void *dest)
{
   size_t nbytes = sizeof(hwe_inst32_t);
   memcpy(dest, &cont->body, nbytes);
   dest += nbytes;

   if (cont->body.jump) {
      uint32_t *tmp = dest;
      *(tmp++) = cont->jump_pc;
      dest = tmp;
   }
   
   nbytes = cont->body.nregs;
   memcpy(dest, &cont->reg_id, nbytes);
   dest += nbytes;
   
   nbytes = 4 * cont->body.nregs;
   memcpy(dest, &cont->reg_data, nbytes);
   return dest + nbytes;
}
static inline void * hwe_excep32_write(const hwe_excep_cont *cont, void *dest)
{
   size_t nbytes = sizeof(hwe_excep32_t);
   memcpy(dest, &cont->body, nbytes);
   return dest + nbytes;
}

static inline void * hwe_cpu32_write(const hwe_cpu_cont *cont, void *dest)
{
   size_t nbytes = sizeof(hwe_cpu32_t);
   memcpy(dest, &cont->body, nbytes);
   return dest + nbytes;
}

/*
 * read
 */
static inline size_t hwe_inst32_read(hwe_inst_cont *cont,
      const void *buf, const size_t size)
{
   size_t need, cur;
   
   cur = sizeof(hwe_inst32_t);
   need = cur;
   if (size < need) return 0;
   memcpy(&cont->body, buf, cur);
   buf += cur;

   cur = cont->body.nregs;
   size_t cur2 = (cont->body.regdata)?(4 * cont->body.nregs):0;
   need += cur + cur2 + (cont->body.jump ? 4 : 0);
   if (size < need) return 0;

   if (cont->body.jump) {
      memcpy(&cont->jump_pc, buf, 4);
      buf += 4;
   }
   memcpy(&cont->reg_id, buf, cur);
   buf += cur;
   memcpy(&cont->reg_data, buf, cur2);

   return need;
}
static inline size_t hwe_excep32_read(hwe_excep_cont *cont,
      const void *buf, const size_t size)
{
   size_t need, cur;
   
   cur = sizeof(hwe_excep32_t);
   need = cur;
   if (size < need) return 0;
   memcpy(&cont->body, buf, cur);
   return need;
}

static inline size_t hwe_cpu32_read(hwe_cpu_cont *cont,
      const void *buf, const size_t size)
{
   size_t need, cur;
   cur = sizeof(hwe_cpu32_t);
   need = cur;
   if (size < need) return 0;
   memcpy(&cont->body, buf, cur);
   return need;
}


/*
 * print
 */
static inline void hwe_inst32_print(FILE *stream, const hwe_inst_cont *cont)
{
   fprintf(stream, "    PC=0x%08"PRIx32" (size:%u)%s\n", cont->body.pc, 1 + cont->body.width, cont->body.abort ? "aborted":"");
   if (cont->body.jump)
      fprintf(stream, "\tJMP 0x%08"PRIx32"\n", cont->jump_pc);
   if (cont->body.regdata)
      for (unsigned int i = 0; i < cont->body.nregs; i++)
         fprintf(stream, "\treg[%u] %s 0x%08"PRIx32"\n", cont->reg_id[i], (cont->body.regdst & (1U << i)) ? "->" : "<-", cont->reg_data[i]);
   else
      for (unsigned int i = 0; i < cont->body.nregs; i++)
         fprintf(stream, "\treg[%u] %s ?\n", cont->reg_id[i], (cont->body.regdst & (1U << i)) ? "->" : "<-");
}
static inline void hwe_excep32_print(FILE *stream, const hwe_excep_cont *cont)
{
   fprintf(stream, "    RETPC=0x%08"PRIx32"\n", cont->body.retpc);
}

static inline void hwe_cpu32_print(FILE *stream, const hwe_cpu_cont *cont)
{
  fprintf(stream,"     ACCESS: %s \n", cont->body.io_mem ? "IO" : "MEM");
}


/*
 * desc
 */
static inline int hwe_inst32_desc(const hwe_inst_cont *cont, char *str, int len)
{
   return snprintf(str, len, " 0x%08"PRIx32"%s%s", cont->body.pc, 
         cont->body.jump ? " J" : "",
         cont->body.abort ? (cont->body.jump ? "A" : " A") : "");
}
static inline int hwe_excep32_desc(const hwe_excep_cont *cont, char *str, int len)
{
   return snprintf(str, len, " 0x%08"PRIx32, cont->body.retpc);
}

static inline int hwe_cpu32_desc(const hwe_cpu_cont *cont, char *str, int len)
{
   return snprintf(str, len, " %s", cont->body.io_mem ? "IO":"MEM");
}



#endif // _HWE_TOOLS_INST_H_
#endif // HWE_USE_TOOLS

