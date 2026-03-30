/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Blinky application
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 *
 * @}
 */

#include <stdio.h>

#include "clk.h"
#include "board.h"
#include "periph_conf.h"

static void delay(void)
{
    /* Busy-wait ~0.5 s; minimum guard keeps blink visible if coreclk() is low. */
    uint32_t loops = coreclk() / 8;
    if (loops < 100000u) {
        loops = 100000u;
    }
    for (volatile uint32_t i = 0; i < loops; i++) { }
}

int main(void)
{
    while (1) {
#ifdef LED0_TOGGLE
        LED0_TOGGLE;
        delay();
#else
        puts("Blink! (No LED present or configured...)");
#endif
    }

    return 0;
}
