
/**
 * @ingroup     boards_bluebirdVG015
 * @{
 */

/**
 * @file
 * @brief       Board init: user LED HL1 
 */

#include <stdint.h>

#include "board.h"
#include "K1921VG015.h"

static void _bsp_led_init(void)
{
    GPIO_TypeDef *gpio = BLUEBIRD_LED_GPIO;
    uint32_t pin = BLUEBIRD_LED_PIN_NUM;
    uint32_t mask = (1U << pin);

    if (gpio == GPIOA) {
        RCU->CGCFGAHB_bit.GPIOAEN = 1;
        RCU->RSTDISAHB_bit.GPIOAEN = 1;
    }
    else if (gpio == GPIOB) {
        RCU->CGCFGAHB_bit.GPIOBEN = 1;
        RCU->RSTDISAHB_bit.GPIOBEN = 1;
    }
    else if (gpio == GPIOC) {
        RCU->CGCFGAHB_bit.GPIOCEN = 1;
        RCU->RSTDISAHB_bit.GPIOCEN = 1;
    }

    gpio->OUTENSET = mask;
    gpio->DATAOUTSET = mask;
}

void board_init(void)
{
    _bsp_led_init();

}

