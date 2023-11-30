#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

typedef struct {
    volatile uint32_t FIQ            :  1;
    volatile uint32_t IRQ            :  1;
    volatile uint32_t not_used       :  1;
    volatile uint32_t undefined      :  1;
    volatile uint32_t prefetch_abort :  1;
    volatile uint32_t data_abort     :  1;
    volatile uint32_t super_call     :  1;
             uint32_t padding        : 25;
} unex_irq_t;

extern unex_irq_t unex_irq;

#endif
