#include <stdio.h>
#include <stdint.h>

#include "typedef.h"
#include "interrupts.h"

unex_irq_t unex_irq;

// Should be actually NOT used but hey, just in case
void _WEAK_ isrNotUsed(void) { unex_irq.not_used = 1; }

void _WEAK_ isrUndefined(void) { unex_irq.undefined = 1; }
void _WEAK_ isrPrftchAbrt(void) { unex_irq.prefetch_abort = 1; }
void _WEAK_ isrDataAbrt(void) { unex_irq.data_abort = 1; }
void _WEAK_ isrSuperCall(void) { unex_irq.super_call = 1; }
void _WEAK_ isrIRQ(void) { unex_irq.IRQ = 1; }
void _WEAK_ isrFIQ(void) { unex_irq.FIQ = 1; }



