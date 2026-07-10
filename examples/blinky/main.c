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
#include "periph/timer.h"


static void timer_callback(void *arg, int channel)
{
    (void)arg;
    (void)channel;
    
    //puts("Таймер сработал!");
    LED0_TOGGLE;
            for(volatile int i = 0; i < 100000; i++){
                volatile int j = i*2;
                (void) j;
            }
}

//void TMR2_millis_delay(uint16_t millis)  {
//  uint32_t iter = 0;
//  while (iter < millis*10) {  
//    timer_start(TIMER_DEV(0));
//    while(timer_read(TIMER_DEV(0)) < 608);
//    timer_stop(TIMER_DEV(0));
//    timer_clear(TIMER_DEV(0), 1);
//    iter++;
//  };
//}

int main(void)
{
    //LED0_TOGGLE;
    timer_init(TIMER_DEV(0), 1000, timer_callback, NULL);
    //LED0_TOGGLE;
    while (1) {
#ifdef LED0_TOGGLE
        //LED0_TOGGLE;
        //TMR2_millis_delay(10);
        //delay();
#else
        puts("Blink! (No LED present or configured...)");
#endif 
        for(volatile int i = 0; i < 100000; i++){
            volatile int j = i*2;
            (void) j;
        }
    }

    return 0;
}
