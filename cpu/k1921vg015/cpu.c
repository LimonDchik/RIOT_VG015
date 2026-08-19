/*
 * Copyright (C) 2025
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1.
 */

#include "cpu.h"
#include "irq_arch.h"
#include "periph/init.h"
#include "stdio_uart.h"
#include "vendor/riscv_csr.h"

void SystemInit(void);
void SystemCoreClockUpdate(void);

static void _cpu_irq_post_init(void)
{
    clear_csr(mie, MIP_MTIP);
}

void cpu_init(void)
{
    SystemInit();
    SystemCoreClockUpdate();
    riscv_init();
    _cpu_irq_post_init();
    stdio_init();
    periph_init();
}
