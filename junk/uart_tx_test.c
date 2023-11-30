#include <stdio.h>
#include <stdint.h>

#include "../main.h"
#include "uart_16550.h"
#include "../arm_lowlev.h"


void PrintChar(char c) {
    while (!(uart->LSR & LSR_TXH_EMPTY))    ;
    uart->mode.operational.RXTX = c;
}

void play_code(void) {
    uint32_t idreg = readCoreID();
    while ( 1 ) {
        printf("Testing override (idreg: %08lx)\r\n", idreg);
    }
}
