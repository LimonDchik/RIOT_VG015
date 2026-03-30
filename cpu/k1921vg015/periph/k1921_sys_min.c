/*
 * Minimal clock / SystemInit for RIOT until vendor system_k1921vg015.c is
 * integrated (that file needs -std=gnu11 and SYSCLK_* build flags).
 */

#include <stdint.h>

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
    SystemCoreClock = HSICLK_VAL;
}

void SystemInit(void)
{
    SystemCoreClockUpdate();
}
