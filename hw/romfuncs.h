#ifndef __ROMCODE_H__
#define __ROMCODE_H__


// ROM function prototypes
typedef void (*puts_t)(const char *str);


#if defined(PCB100466824)
#include "PCB/PCB100466824.h"
#elif defined(PCB100434407)
#include "PCB/PCB100434407.h"
#else
#error "Specify PCB number"
#endif

#endif
