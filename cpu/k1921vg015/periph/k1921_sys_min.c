/*
 * Minimal clock / SystemInit for RIOT until vendor system_k1921vg015.c is
 * integrated (that file needs -std=gnu11 and SYSCLK_* build flags).
 */

#include <stdint.h>

#include "K1921VG015.h"

#ifndef HSICLK_VAL
#define HSICLK_VAL (1000000U)
#endif

uint32_t SystemCoreClock;
uint32_t SystemPll0Clock;
uint32_t SystemPll1Clock;
uint32_t USBClock;

void SystemCoreClockUpdate(void)
{
    SystemPll0Clock = 0;
    SystemPll1Clock = 0;
    USBClock = 0;
    switch ((RCU->CLKSTAT & RCU_CLKSTAT_SRC_Msk) >>
            RCU_CLKSTAT_SRC_Pos) {
        case RCU_CLKSTAT_SRC_HSICLK:
            SystemCoreClock = HSICLK_VAL;
            break;
        case RCU_CLKSTAT_SRC_LSICLK:
            SystemCoreClock = 32768U;
            break;
        default:
            SystemCoreClock = 0;
            break;
    }
}

void SystemInit(void)
{
    RCU->SYSCLKCFG = RCU_SYSCLKCFG_SRC_HSICLK << RCU_SYSCLKCFG_SRC_Pos;
    while (((RCU->CLKSTAT & RCU_CLKSTAT_SRC_Msk) >>
            RCU_CLKSTAT_SRC_Pos) != RCU_CLKSTAT_SRC_HSICLK) {
    }

    SystemCoreClockUpdate();
}
