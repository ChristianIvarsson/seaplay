#include <stdio.h>
#include <stdint.h>

#include "typedef.h"
#include "interrupts.h"

#include "arm_lowlev.h"

unex_irq_t unex_irq = { 0 };

// Should be actually NOT used but hey, just in case
void _WEAK_ isrNotUsed(void) {
    static uint32_t count = 0;
    unex_irq.not_used = 1;
    if (count++ >= 10) disableAll();
}

void _WEAK_ isrUndefined(void) {
    static uint32_t count = 0;
    unex_irq.undefined = 1;
    if (count++ >= 10) disableAll();
}
void _WEAK_ isrPrftchAbrt(void) {
    static uint32_t count = 0;
    unex_irq.prefetch_abort = 1;
    if (count++ >= 10) disableAll();
}
void _WEAK_ isrDataAbrt(void) {
    static uint32_t count = 0;
    unex_irq.data_abort = 1;
    if (count++ >= 10) disableAll();
}
void _WEAK_ isrSuperCall(void) {
    static uint32_t count = 0;
    unex_irq.super_call = 1;
    if (count++ >= 10) disableAll();
}
void _WEAK_ isrIRQ(void) {
    static uint32_t count = 0;
    unex_irq.IRQ = 1;
    if (count++ >= 10) disableAll();
}
void _WEAK_ isrFIQ(void) {
    static uint32_t count = 0;
    unex_irq.FIQ = 1;
    if (count++ >= 10) disableAll();
}



