#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIMER_NUMOF         (1U)
#ifndef RTC_FREQ
#define RTC_FREQ            (1000000UL)
#endif
/** @} */

static const uart_conf_t uart_config[] = {
    { .dev = UART3 },
};

#define UART_NUMOF (sizeof(uart_config) / sizeof(uart_config[0]))

/**
 * @name   Timer configuration
 * @{
 */
static const timer_conf_t timer_config[] = {
    {
        .dev      = TMR1,
        .max      = 0x0000ffff,
        .irqn     = IsrVect_IRQ_TMR1
    }
};

#define TMR1_IRQN        1

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */


