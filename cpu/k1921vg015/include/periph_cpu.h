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

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */

