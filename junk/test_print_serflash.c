#include <stdio.h>
#include <stdint.h>

#include "../main.h"
#include "../interrupts.h"
#include "../arm_lowlev.h"
#include "uart_16550.h"

typedef uint16_t (*readSpi_t)(uint32_t unk, void *buf, uint32_t flashAddress, uint32_t nDwords, uint32_t partIdx);

void PrintChar(char c) {
    while (!(uart->LSR & LSR_TXH_EMPTY))    ;
    uart->mode.operational.RXTX = c;
}

char ReadChar(void) {
    while (!(uart->LSR & LSR_TXH_EMPTY))    ;
    return uart->mode.operational.RXTX;
}

void playInterrupts(void) {
    // setExceptionMask( EXCMSK_NONE );
    // enableAll();
    enableFIQ();

    // Enable RX int flag
    uart->mode.operational.IER = 1;
}

void play_code(void) {

    // initVec();
    // playInterrupts();
    // id = 41259661

    readSpi_t readSpi = (readSpi_t)(0x10045C + 1);
    uint32_t address = 0;
    uint32_t checksum = 0;
    uint32_t idreg = readCoreID();
    uint8_t buf[32];

    printf("Testing override (IRQ flags: 0x%lx) (idreg: %lx)\r\n", *(volatile uint32_t*)&unex_irq, idreg);

    while (address < (512 * 1024)) {

        readSpi(0, buf, address, 16 / 4, 0);

        for (size_t i = 0; i < 16; i++) {
            printf("%02x ", buf[i]);
            checksum += buf[i];
        }

        printf("\r\n");
        address+=16;
    }

    printf("Flash checksum: %08lx\r\n", checksum);

    while ( 1 ) { }
}
