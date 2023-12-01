#include <stdio.h>
#include <stdint.h>

#include "../main.h"
#include "uart_16550.h"
#include "../arm_lowlev.h"

void PrintChar(char c) {
    while (!(uart->LSR & LSR_TXH_EMPTY))    ;
    uart->mode.operational.RXTX = c;
}

char ReadChar(void) {
    while (!(uart->LSR & LSR_RX_READY))    ;
    return uart->mode.operational.RXTX;
}

void play_code(void) {
    uint32_t idreg = readCoreID();
    while ( 1 ) {
        printf("Testing override (idreg: %08lx)\r\n", idreg);
    }
}
