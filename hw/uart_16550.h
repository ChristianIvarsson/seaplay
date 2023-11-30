#ifndef __UART_16550_H__
#define __UART_16550_H__

#include "MCU/mcu.h"

typedef struct {

    union {
        struct {
        // (0) RX / TX register
        __IO uint32_t RXTX;

        // (1) Interrupt enable register
        // 7:4 - - - - - - - - -
        //   3 - Modem status interrupt
        //   2 - Receive line status interrupt
        //   1 - Transmit holding register
        //   0 - Receive holding register
        __IO uint32_t IER;
        } operational;

        struct {
        // (0) Baud divisor LSB
        __IO uint32_t DLL;

        // (1) Baud divisor MSB
        __IO uint32_t DLM;
        } config;
    } mode;

    // (2) < Read > ISR - Interrupt status flags
    //   7 - FIFOs enabled
    //   6 - FIFOs enabled
    // 5:4 - - - - - - - - -
    // 3:1 - INT prio bits
    // - - - 3: LSR   - (p1) Receiver Line Status Register
    // - - - 2: RXRDY - (p2) Received data
    // - - - 6: RXRDY - (p2) Receive data timeout
    // - - - 1: TXRDY - (p3) TX register empty
    // - - - 0: MSR   - (p4) Modem status register
    //   0 - Int valid. (0 == valid)

    // (2) < Write > FCR - FIFO flags
    // Set "FCR_EN_FIFO" _BEFORE_ setting the other flags
    // 7:6 - RX FIFO trigger level
    // 5:4 - - - - - - - - -
    //   3 - DMA mode select
    //   2 - TX FIFO reset
    //   1 - RX FIFO reset
    //   0 - FIFO enable  (FCR_EN_FIFO)
    __IO uint32_t ISR_FCR;


    // (3) Line Control Register
    //   7 - Divisor latch enable
    //   6 - Set break
    //   5 - Forced parity
    //   4 - Even parity
    //   3 - Parity enable
    // 2:0 - Bit config
    // - - - 7: 8 bits, 2 stop bits
    // - - - 6: 7 bits, 2 stop bits
    // - - - 5: 6 bits, 2 stop bits
    // - - - 4: 5 bits, 1.5 stop bits
    // - - - 3: 8 bits, 1 stop bit
    // - - - 2: 7 bits, 1 stop bit
    // - - - 1: 6 bits, 1 stop bit
    // - - - 0: 5 bits, 1 stop bit
    __IO uint32_t LCR;

    // (4) Modem Control Register
    // 7:6 - - - - - - - - -
    //   5 - Automatic flow control
    //   4 - Loopback mode
    //   3 - _OUT2_ interrupt enable
    //   2 - _OUT1_
    //   1 - _RTS_
    //   0 - _DTR_
    __IO uint32_t MCR;

    // (5) Line Status Register
    //   7 - FIFO data error
    //   6 - TX empty
    //   5 - TX holding empty
    //   4 - Break interrupt
    //   3 - Framing Error
    //   2 - Parity error
    //   1 - Overrun error
    //   0 - RX ready
    __IO const uint32_t LSR;

    // (6) Modem Status Register
    //   7 - DCD
    //   6 - RI
    //   5 - DSR
    //   4 - CTS
    //   3 - _DCD_
    //   2 - _RI_
    //   1 - _DSR_
    //   0 - _CTS_
    __IO const uint32_t MSR;

    // (7) Scratch register
    __IO uint32_t SPR;

} uart16550_t;

#define uart       ((uart16550_t *)    UART_BASE)

// FCR flags
#define FCR_FIFOTRIG_1      (0x00000000)  // Trigger at one byte
#define FCR_FIFOTRIG_4      (0x00000040)  // Trigger at four bytes
#define FCR_FIFOTRIG_8      (0x00000080)  // Trigger at eight bytes
#define FCR_FIFOTRIG_14     (0x000000C0)  // Trigger at fourteen bytes
#define FCR_FIFOTRIG_MASK   (0x000000C0)

#define FCR_MODE            (0x00000008)  // Toggle RXRDY/TXRDY pins mode.
#define FCR_FLUSH_TX        (0x00000004)  // Reset TX FIFO
#define FCR_FLUSH_RX        (0x00000002)  // Reset RX FIFO
#define FCR_EN_FIFO         (0x00000001)  // Enable FIFO operation

#define LSR_TXH_EMPTY       (0x00000020)

#endif
