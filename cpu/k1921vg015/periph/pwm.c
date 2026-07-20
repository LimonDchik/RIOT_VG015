/*
 * Copyright (C) 2026
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_k1921vg015
 * @ingroup     drivers_periph_pwm
 * @{
 *
 * @file
 * @brief       Low-level PWM driver for K1921VG015
 *
 * @}
 */

#include <assert.h>
#include <stdint.h>

#include "cpu.h"
#include "periph/pwm.h"

extern uint32_t SystemCoreClock;

void SystemCoreClockUpdate(void);

typedef struct {
    uint16_t resolution;
    pwm_mode_t mode;
} pwm_state_t;

static pwm_state_t _state[PWM_NUMOF];

static inline TMR_TypeDef *_dev(pwm_t pwm)
{
    return pwm_config[pwm].dev;
}

static void _gpio_init(const pwm_chan_conf_t *channel)
{
    uint32_t shift = 2U * channel->pin;
    uint32_t mask = 3UL << shift;

    channel->port->ALTFUNCNUM = (channel->port->ALTFUNCNUM & ~mask) |
                               ((uint32_t)channel->af << shift);
    channel->port->ALTFUNCSET = 1UL << channel->pin;
}

static uint32_t _select_divider(uint32_t target, uint32_t *divider)
{
    static const uint8_t divisors[] = { 1, 2, 4, 8 };

    for (unsigned i = 0; i < sizeof(divisors); ++i) {
        uint32_t clock = SystemCoreClock / divisors[i];

        if (clock <= target) {
            *divider = i;
            return clock;
        }
    }

    *divider = TMR_CTRL_DIV_Div8;
    return SystemCoreClock / 8U;
}

uint32_t pwm_init(pwm_t pwm, pwm_mode_t mode, uint32_t freq, uint16_t res)
{
    if ((pwm >= PWM_NUMOF) || (freq == 0U) || (res < 2U) ||
        (mode > PWM_CENTER)) {
        return 0;
    }

    SystemCoreClockUpdate();
    if (SystemCoreClock == 0U) {
        return 0;
    }

    uint32_t period_factor = (mode == PWM_CENTER) ? 2U : 1U;
    uint64_t requested_clock = (uint64_t)freq * res * period_factor;
    uint32_t target = (requested_clock > UINT32_MAX) ? UINT32_MAX :
                      (uint32_t)requested_clock;
    uint32_t divider;
    uint32_t timer_clock = _select_divider(target, &divider);
    TMR_TypeDef *timer = _dev(pwm);

    RCU->CGCFGAPB |= pwm_config[pwm].clock_mask;
    RCU->RSTDISAPB |= pwm_config[pwm].reset_mask;

    timer->CTRL = 0;
    timer->IM = 0;
    timer->DMA_IM = 0;
    timer->ADC_IM = 0;
    timer->COUNT = 0;
    for (unsigned i = 0; i < 4; ++i) {
        timer->CAPCOM[i].CTRL = 0;
        timer->CAPCOM[i].VAL = 0;
    }

    timer->CAPCOM[0].VAL = res - 1U;
    timer->CTRL_bit.DIV = divider;
    timer->CTRL_bit.CLKSEL = TMR_CTRL_CLKSEL_SysClk;
    timer->CTRL_bit.CLR = 1;
    timer->CTRL_bit.CLR = 0;

    for (unsigned i = 0; i < pwm_config[pwm].channel_numof; ++i) {
        _gpio_init(&pwm_config[pwm].channels[i]);
    }

    _state[pwm].resolution = res;
    _state[pwm].mode = mode;

    timer->CTRL_bit.MODE = (mode == PWM_CENTER) ? TMR_CTRL_MODE_UpDown :
                                                 TMR_CTRL_MODE_Up;

    return timer_clock / ((uint32_t)res * period_factor);
}

uint8_t pwm_channels(pwm_t pwm)
{
    assert(pwm < PWM_NUMOF);
    return pwm_config[pwm].channel_numof;
}

void pwm_set(pwm_t pwm, uint8_t channel, uint16_t value)
{
    assert((pwm < PWM_NUMOF) &&
           (channel < pwm_config[pwm].channel_numof));

    const pwm_chan_conf_t *conf = &pwm_config[pwm].channels[channel];
    _TMR_CAPCOM_TypeDef *capcom = &_dev(pwm)->CAPCOM[conf->channel];
    uint16_t resolution = _state[pwm].resolution;

    if (value > resolution) {
        value = resolution;
    }

    /* Comparator modes can produce a narrow glitch at the end points. */
    if ((value == 0U) || (value >= (resolution - 1U))) {
        capcom->CTRL = (value != 0U) ? TMR_CAPCOM_CTRL_OUT_Msk : 0U;
        return;
    }

    if (_state[pwm].mode == PWM_RIGHT) {
        capcom->VAL = (resolution - 1U) - value;
        capcom->CTRL = TMR_CAPCOM_CTRL_OUTMODE_Set_Reset <<
                       TMR_CAPCOM_CTRL_OUTMODE_Pos;
    }
    else if (_state[pwm].mode == PWM_CENTER) {
        capcom->VAL = (resolution - 1U) - value;
        capcom->CTRL = TMR_CAPCOM_CTRL_OUTMODE_Toggle_Set <<
                       TMR_CAPCOM_CTRL_OUTMODE_Pos;
    }
    else {
        capcom->VAL = value;
        capcom->CTRL = TMR_CAPCOM_CTRL_OUTMODE_Reset_Set <<
                       TMR_CAPCOM_CTRL_OUTMODE_Pos;
    }
}

void pwm_poweron(pwm_t pwm)
{
    assert(pwm < PWM_NUMOF);
    RCU->CGCFGAPB |= pwm_config[pwm].clock_mask;
    _dev(pwm)->CTRL_bit.MODE = (_state[pwm].mode == PWM_CENTER) ?
                               TMR_CTRL_MODE_UpDown : TMR_CTRL_MODE_Up;
}

void pwm_poweroff(pwm_t pwm)
{
    assert(pwm < PWM_NUMOF);
    _dev(pwm)->CTRL_bit.MODE = TMR_CTRL_MODE_Stop;
    RCU->CGCFGAPB &= ~pwm_config[pwm].clock_mask;
}
