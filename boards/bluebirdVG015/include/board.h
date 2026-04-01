#ifndef BOARD_H
#define BOARD_H

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    STDIO UART
 * @{
 */
#define STDIO_UART_DEV      UART_DEV(0)
#define STDIO_UART_BAUDRATE (115200U)

#ifndef BLUEBIRD_LED_GPIO
#define BLUEBIRD_LED_GPIO   GPIOC
#endif

#ifndef BLUEBIRD_LED_PIN_NUM
#define BLUEBIRD_LED_PIN_NUM    (0U)
#endif

#ifndef BLUEBIRD_LED_ACTIVE_LOW
#define BLUEBIRD_LED_ACTIVE_LOW   1
#endif

#if BLUEBIRD_LED_ACTIVE_LOW
#define LED0_ON   do { (BLUEBIRD_LED_GPIO)->DATAOUT &= ~(1U << (BLUEBIRD_LED_PIN_NUM)); } while (0)
#define LED0_OFF  do { (BLUEBIRD_LED_GPIO)->DATAOUT |= (1U << (BLUEBIRD_LED_PIN_NUM)); } while (0)
#else
#define LED0_ON   do { (BLUEBIRD_LED_GPIO)->DATAOUT |= (1U << (BLUEBIRD_LED_PIN_NUM)); } while (0)
#define LED0_OFF  do { (BLUEBIRD_LED_GPIO)->DATAOUT &= ~(1U << (BLUEBIRD_LED_PIN_NUM)); } while (0)
#endif
#define LED0_TOGGLE  ((void)((BLUEBIRD_LED_GPIO)->DATAOUTTGL = (1U << (BLUEBIRD_LED_PIN_NUM))))
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
