#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIMER_NUMOF         (1U)
#define TIMER_0_MAX_VALUE   (0xffffffffUL)
#ifndef RTC_FREQ
#define RTC_FREQ            (1000000UL)
#endif
#ifndef CLOCK_CORECLOCK
#define CLOCK_CORECLOCK     RTC_FREQ
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
        .dev      = TMR32,
        .max      = TIMER_0_MAX_VALUE,
        .irqn     = IsrVect_IRQ_TMR32
    }
};

#define XTIMER_HZ           RTC_FREQ

/**
 * @name   PWM configuration
 *
 * TMR0 CAPCOM0 defines the PWM period.  The three remaining compare channels
 * are routed to TMR0_OUT1..3 on PC7..PC9 using alternate function 2.
 * @{
 */
static const pwm_chan_conf_t pwm0_channels[] = {
    { .port = GPIOC, .pin = 7, .channel = 1, .af = 2 },
    { .port = GPIOC, .pin = 8, .channel = 2, .af = 2 },
    { .port = GPIOC, .pin = 9, .channel = 3, .af = 2 },
};

/* PA9 alternate function 2 is TMR1_OUT3. */
static const pwm_chan_conf_t pwm1_channels[] = {
    { .port = GPIOA, .pin = 9, .channel = 3, .af = 2 },
};

static const pwm_conf_t pwm_config[] = {
    {
        .dev = TMR0,
        .channels = pwm0_channels,
        .channel_numof = sizeof(pwm0_channels) / sizeof(pwm0_channels[0]),
        .clock_mask = RCU_CGCFGAPB_TMR0EN_Msk,
        .reset_mask = RCU_RSTDISAPB_TMR0EN_Msk,
    },
    {
        .dev = TMR1,
        .channels = pwm1_channels,
        .channel_numof = sizeof(pwm1_channels) / sizeof(pwm1_channels[0]),
        .clock_mask = RCU_CGCFGAPB_TMR1EN_Msk,
        .reset_mask = RCU_RSTDISAPB_TMR1EN_Msk,
    },
};

#define PWM_NUMOF           (sizeof(pwm_config) / sizeof(pwm_config[0]))
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
