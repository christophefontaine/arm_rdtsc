#ifndef _ARM_ASM_H_
#define _ARM_ASM_H_

#if defined(__ARM_ARCH_8A__) /* we may run a 64bits processor in 32bits mode */
inline uint64_t cpu_time_now (void)
{
  uint64_t tsc;
  asm volatile("mrrc p15, 0, %Q0, %R0, c9" : "=r" (tsc));
  return tsc;
}
#elif defined(__ARM_ARCH_7A__)
inline uint64_t cpu_time_now (void)
{
  uint32_t tsc;
  asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r" (tsc));
  return (uint64_t)tsc;
}
#else
inline uint64_t cpu_time_now (void)
{
  uint32_t lo;
  asm volatile ("mrc p15, 0, %[lo], c15, c12, 1"
                : [lo] "=r" (lo));
  return (uint64_t) lo;
}
#endif


uint32_t _cpsr(void) {
  uint32_t cpsr;
  asm volatile ("mrs %0, CPSR" : "=r"(cpsr));
  return cpsr;
}

uint32_t _hdcr(void) { /*  Hyp Debug Configuration Register */
  uint32_t hdcr;
  asm volatile("mrc p15, 4, %0, c1, c1, 1" : "=r"(hdcr));
  return hdcr;
}

uint32_t _scr(void) {
  uint32_t scr;
  asm volatile("mrc p15, 0, %0, c1, c1, 0" : "=r"(scr));
  return scr;
}

uint32_t _pmcr(void) {
  uint32_t pmcr;
  asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr));
  return pmcr;
}

void _pmcr_set(uint32_t pmcr) {
  asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(pmcr));
}

uint32_t _pmuseren(void) {
  uint32_t en;
  asm volatile ("mrc p15, 0, %0, c9, c14, 0 " : "=r"(en));
  return en;
}

#define MAJOR_NUM 0
#define DEVICE_NAME "rdtsc"


struct dma_buf_t {
        size_t len; // requested length
        void* vaddr; // virtual address
        dma_addr_t paddr; // associated physical addr
};

#define FLUSH_CACHE  _IO(MAJOR_NUM, 1)
#define GET_DMA_BUF  _IOR(MAJOR_NUM, 2, struct dma_buf_t*)
#define SET_UNCACHED  _IOR(MAJOR_NUM, 3, struct dma_buf_t*)


#endif // _ARM_ASM_H_
