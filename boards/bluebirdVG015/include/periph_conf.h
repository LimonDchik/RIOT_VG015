#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Timer configuration
 * @{
 */
#define TIMER_NUMOF         (1U)
#define TIMER_0_MAX_VALUE   (0xffffffffUL)

#ifndef RTC_FREQ
#define RTC_FREQ            (1000000UL)
#endif

#ifndef CLOCK_CORECLOCK
#define CLOCK_CORECLOCK     RTC_FREQ
#endif

/**
 * @name UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    { .dev = UART3 },
};
#define UART_NUMOF (sizeof(uart_config) / sizeof(uart_config[0]))
/** @} */

static const timer_conf_t timer_config[] = {
    {
        .dev      = TMR32,
        .max      = TIMER_0_MAX_VALUE,
        .irqn     = IsrVect_IRQ_TMR32
    }
};

#define XTIMER_HZ           RTC_FREQ
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
