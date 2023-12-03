
#ifndef __ARM_LOWLEV_H__
#define __ARM_LOWLEV_H__

typedef struct {
    uint32_t min_rev   : 4;   // 03:00   - Minor revision
    uint32_t part      :12;   // 15:04   - Part number
    uint32_t arch      : 4;   // 19:16   - Architecture
    uint32_t maj_rev   : 4;   // 23:20   - Major revision
    uint32_t impl      : 8;   // 31:24   - Implementer
} armid_t;

typedef struct {
    uint32_t res_0     : 2;   // 01:00   - Reserved
    uint32_t itcm_abs  : 1;   //    02   - ITCM absent
    uint32_t res_3     : 3;   // 05:03   - Reserved
    uint32_t itcm_sz   : 5;   // 10:06   - ITCM size
    uint32_t res_11    : 3;   // 13:11   - Reserved
    uint32_t dtcm_abs  : 1;   //    14   - DTCM absent
    uint32_t res_15    : 3;   // 17:15   - Reserved
    uint32_t dtcm_sz   : 5;   // 22:18   - DTCM size
    uint32_t res_23    : 9;   // 31:23
} tcm_t;

static inline armid_t getCoreInfo(void) {
    armid_t retVal;
    asm volatile( "MRC p15, 0, %0, c0, c0, 0\n" : "=r" (retVal) );
    return retVal;
}

static inline tcm_t getTCMInfo(void) {
    tcm_t retVal;
    asm volatile( "MRC p15, 0, %0, c0, c0, 2\n" : "=r" (retVal) );
    return retVal;
}






// Everything after this line is experimental garbage!

#define EXCMSK_NONE   (0)
#define EXCMSK_FIQ    (1)
#define EXCMSK_IRQ    (2)
#define EXCMSK_BOTH   (3)
/*
static inline void setExceptionMask(uint32_t msk) {
    msk = (msk & 3) << 6;
    asm volatile(
        "mrs             r1, CPSR         \n"
        "mov             r2, #0xFFFFFF3F  \n"
        "and             r1, r2           \n"
        "orr             r1, %0           \n"
        "msr             CPSR, r1         \n"
        :: "r" (msk)   // Input
        : "r1", "r2"); // Clobbers
}
*/

static inline void enableAll(void) {
    asm volatile(
        "mov             r0, #0x13  \n"
        "msr             CPSR_c, r0 \n"
        ::: "r0"); // Clobbers
}

static inline void enableIRQ(void) {
    asm volatile(
        "mov             r0, #0x53  \n"
        "msr             CPSR_c, r0 \n"
        ::: "r0"); // Clobbers
}

static inline void enableFIQ(void) {
    asm volatile(
        "mov             r0, #0x93  \n"
        "msr             CPSR_c, r0 \n"
        ::: "r0"); // Clobbers
}

static inline void disableAll(void) {
    asm volatile(
        "mov             r0, #0xd3    \n"
        "msr             CPSR_c, r0 \n"
        ::: "r0"); // Clobbers
}

#endif
