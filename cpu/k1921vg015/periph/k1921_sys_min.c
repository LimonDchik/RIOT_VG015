/*
 * Minimal 50 MHz clock setup for the BlueBird-VG015 board.
 */

#include <stdint.h>

#include "K1921VG015.h"

#ifndef HSICLK_VAL
#define HSICLK_VAL (1000000U)
#endif

#define HSECLK_VAL         (16000000U)
#define PLL0CLK_VAL        (50000000U)

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
        case RCU_CLKSTAT_SRC_HSECLK:
            SystemCoreClock = HSECLK_VAL;
            break;
        case RCU_CLKSTAT_SRC_SYSPLL0CLK:
            SystemCoreClock = PLL0CLK_VAL;
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
    /* Configure PLL while the core is running from the 16 MHz HSE. */
    RCU->SYSCLKCFG = RCU_SYSCLKCFG_SRC_HSECLK << RCU_SYSCLKCFG_SRC_Pos;
    while (((RCU->CLKSTAT & RCU_CLKSTAT_SRC_Msk) >>
            RCU_CLKSTAT_SRC_Pos) != RCU_CLKSTAT_SRC_HSECLK) {
    }

    /* 16 MHz * 100 / (1 * (7 + 1) * (3 + 1)) = 50 MHz. */
    RCU->PLLSYSCFG0 =
        (15U << RCU_PLLSYSCFG0_PD1B_Pos) |
        (3U << RCU_PLLSYSCFG0_PD1A_Pos) |
        (3U << RCU_PLLSYSCFG0_PD0B_Pos) |
        (7U << RCU_PLLSYSCFG0_PD0A_Pos) |
        (1U << RCU_PLLSYSCFG0_REFDIV_Pos) |
        (3U << RCU_PLLSYSCFG0_BYP_Pos) |
        RCU_PLLSYSCFG0_PLLEN_Msk;
    RCU->PLLSYSCFG1 = 0;
    RCU->PLLSYSCFG2 = 100U;
    RCU->PLLSYSCFG0_bit.FOUTEN = 1;

    for (volatile uint32_t delay = 1000U; delay > 0U; delay--) {
    }
    while (RCU->PLLSYSSTAT_bit.LOCK == 0U) {
    }

    RCU->PLLSYSCFG0_bit.BYP = 2;
    FLASH->CTRL_bit.LAT = 3;
    FLASH->CTRL_bit.CEN = 1;

    RCU->SYSCLKCFG = RCU_SYSCLKCFG_SRC_SYSPLL0CLK << RCU_SYSCLKCFG_SRC_Pos;
    while (((RCU->CLKSTAT & RCU_CLKSTAT_SRC_Msk) >>
            RCU_CLKSTAT_SRC_Pos) != RCU_CLKSTAT_SRC_SYSPLL0CLK) {
    }

    SystemCoreClockUpdate();
}
