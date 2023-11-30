#include <stdio.h>
#include <stdint.h>

#include "../main.h"
#include "uart_16550.h"

void PrintChar(char c) {
    while (!(uart->LSR & LSR_TXH_EMPTY))   ;
    uart->mode.operational.RXTX = c;
}

void play_code(void) {
    while ( 1 ) {
        puts( "Testing override\r\n");
    }
}
