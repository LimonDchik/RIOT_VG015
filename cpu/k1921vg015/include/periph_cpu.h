#ifndef PERIPH_CPU_H
#define PERIPH_CPU_H

#include <stdint.h>

#include "cpu.h"
#include "K1921VG015.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   UART hardware descriptor (ARM PL011-compatible block on K1921VG015)
 */
typedef struct {
    UART_TypeDef *dev;      /**< UART register block */
} uart_conf_t;

/**
 * @brief   K1921VG015 timers provide 4 capture/compare channels
 */
#define TIMER_CHANNEL_NUMOF (4U)

/**
 * @brief   The driver provides a relative set function
 */
#define PERIPH_TIMER_PROVIDES_SET

/**
 * @brief   Timer configuration
 */
typedef struct {
    TMR32_TypeDef *dev;     /**< timer device */
    uint32_t max;           /**< maximum value to count to (16/32 bit) */
    uint8_t irqn;           /**< global IRQ channel */
} timer_conf_t;

/**
 * @brief   PWM channel configuration
 *
 * CAPCOM channel 0 is used as the timer period register.  Consequently a
 * PWM device can expose at most CAPCOM channels 1 through 3.
 */
typedef struct {
    GPIO_TypeDef *port;
    uint8_t pin;  
    uint8_t channel;
    uint8_t af;
} pwm_chan_conf_t;

/**
 * @brief   PWM device configuration
 */
typedef struct {
    TMR_TypeDef *dev;
    const pwm_chan_conf_t *channels;
    uint8_t channel_numof;
    uint32_t clock_mask;
    uint32_t reset_mask;
} pwm_conf_t;

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */
