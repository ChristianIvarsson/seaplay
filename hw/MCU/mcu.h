#ifndef __MCU_H__
#define __MCU_H__

#define   __IO  volatile

#if defined (LSI_TTB5501D)
#include "LSI_TTB5501D.h"

// Rename when it's known
#elif defined (LSI_RUE)
#include "LSI_RUE.h"
#else
#error "Specify chip!"
#endif


#ifndef UART_BASE
#define UART_BASE (0xffffffff)
#endif


#endif
