#include <stdio.h>
#include <stdint.h>

#include "typedef.h"
#include "main.h"

// While sorting out the PCB, MCU etc situation you can optionally define the pcb from here

// #define PCB100434407

// #define PCB100466824
//   #define ROM_2_0

#include "hw/romfuncs.h"


extern const int _s_appBase, _e_appBase;
extern const int _sbss, _ebss;

void _WEAK_ play_code(void) {}


// Install vectors where they belong
// LDR PC, =ptr will place hidden data after the code so it's unsafe to use a simple count. - Better fetch locations from the linker script
void initVec(void) {
    const uint32_t *src = (uint32_t*)&_s_appBase;
    uint32_t *dst = 0;
    while ((uint32_t)src < (uint32_t)&_e_appBase) {
        *dst++ = *src++;
    }
}

void initData(void) {
    uint8_t *dst = (uint8_t*)&_sbss;

    // Clear BSS
    while ((uint32_t)dst < (uint32_t)&_ebss) {
        *dst++ = 0;
    }
}

int mainEntry(void) {

    initData();

    play_code();

    // Verified working but currently unused
    // initVec();

    while ( 1 ) {
        puts( "Hello world\r\n" );
    }

    return 0;
}
