#include <stdio.h>
#include <stdint.h>

#include "../main.h"
#include "uart_16550.h"

inline void uartPutChar(const unsigned char ch) {
    while (!(uart->LSR & LSR_TXH_EMPTY))   ;
    uart->mode.operational.RXTX = ch;
}

int puts(const char *s) {
    while (*s)
        uartPutChar((unsigned char)*s++);
    return 0;
}

void play_code(void) {
    while ( 1 ) {
        puts( "Testing override\r\n");
    }
}




