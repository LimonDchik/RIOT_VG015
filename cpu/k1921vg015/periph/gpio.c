/*
 * GPIO stub for k1921vg015 - manual GPIO initialization in board.c
 */

#include "periph/gpio.h"

int gpio_init(gpio_t pin, gpio_mode_t mode)
{
    (void)pin;
    (void)mode;
    /* GPIO is initialized manually in board.c, this is just a stub */
    return 0;
}
