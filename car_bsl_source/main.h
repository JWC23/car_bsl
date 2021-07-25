#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include "NUC029xAN.h"
#include "targetdev.h"
#include "gpio.h"
#include "utils.h"



#define PLLCON_SETTING  CLK_PLLCON_50MHz_HIRC
#define PLL_CLOCK       50000000


#define HEX2ASCII(x)        ( ((x)<=9) ? ((x)+48) : ((x)+55) )


volatile extern uint8_t g_u8UartIdleCntr;


#endif
