/*
 * Copyright (C) 2025
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1.
 */

#include "cpu.h"
#include "irq_arch.h"
#include "periph/init.h"
#include "plic.h"
#include "stdio_uart.h"
#include "vendor/riscv_csr.h"

void SystemInit(void);
void SystemCoreClockUpdate(void);

extern uint32_t SystemCoreClock;

uint32_t cpu_coreclk;

static void _cpu_irq_post_init(void)
{
    clear_csr(mie, MIP_MTIP);
}

void cpu_init(void)
{
    SystemInit();
    SystemCoreClockUpdate();
    cpu_coreclk = SystemCoreClock;
    riscv_init();
    _cpu_irq_post_init();
#ifndef K1921VG015_SKIP_STDIO_INIT
    stdio_init();
#endif
    periph_init();
}
