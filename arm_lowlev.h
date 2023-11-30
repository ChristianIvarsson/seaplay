
#ifndef __ARM_LOWLEV_H__
#define __ARM_LOWLEV_H__

static inline uint32_t readCoreID(void) {
    uint32_t retVal;
    asm volatile( "MRC p15, 0, %0, c0, c0, 0\n" : "=r" (retVal) );
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
