/*
 * Copyright (C) 2025
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1.
 */

#include "cpu.h"
#include "periph/init.h"
#include "stdio_uart.h"

void SystemInit(void);
void SystemCoreClockUpdate(void);

extern uint32_t SystemCoreClock;

uint32_t cpu_coreclk;

void cpu_init(void)
{
    SystemInit();
    SystemCoreClockUpdate();
    cpu_coreclk = SystemCoreClock;
    riscv_init();
#ifndef K1921VG015_SKIP_STDIO_INIT
    stdio_init();
#endif
    periph_init();
}
