/*
 * Copyright (C) 2025
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1.
 */

#include "K1921VG015.h"
#include "periph/pm.h"

void pm_reboot(void)
{
    /* Use the APB watchdog as a best-effort full-chip reset source. */
    RCU->CGCFGAPB_bit.WDTEN = 1;
    RCU->RSTDISAPB_bit.WDTEN = 1;

    WDT->LOCK = 0;
    WDT->LOAD = 1;
    WDT->CTRL = WDT_CTRL_RESEN_Msk;

    while (1) {
    }
}
