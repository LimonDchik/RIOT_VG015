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
    uint32_t ctrl;
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

    return 0;
}

static void _set_outmode(_TMR_CAPCOM_TypeDef *capcom, uint32_t mode)
{
    uint32_t ctrl = capcom->CTRL;
    uint32_t old_mode = (ctrl & TMR_CAPCOM_CTRL_OUTMODE_Msk) >>
                        TMR_CAPCOM_CTRL_OUTMODE_Pos;

    /* Use mode 7 as the transition state recommended by the reference
     * manual when switching directly between two non-zero output modes. */
    if ((old_mode != TMR_CAPCOM_CTRL_OUTMODE_BitOUT) &&
        (mode != TMR_CAPCOM_CTRL_OUTMODE_BitOUT) &&
        (old_mode != mode)) {
        capcom->CTRL = (ctrl & ~TMR_CAPCOM_CTRL_OUTMODE_Msk) |
                       (TMR_CAPCOM_CTRL_OUTMODE_Reset_Set <<
                        TMR_CAPCOM_CTRL_OUTMODE_Pos);
        ctrl = capcom->CTRL;
    }

    capcom->CTRL = (ctrl & ~TMR_CAPCOM_CTRL_OUTMODE_Msk) |
                   (mode << TMR_CAPCOM_CTRL_OUTMODE_Pos);
}

uint32_t pwm_init(pwm_t pwm, pwm_mode_t mode, uint32_t freq, uint16_t res)
{
    if ((pwm >= PWM_NUMOF) || (freq == 0U) || (res < 2U) ||
        ((mode != PWM_LEFT) && (mode != PWM_RIGHT) &&
         (mode != PWM_CENTER))) {
        return 0;
    }

    SystemCoreClockUpdate();
    if (SystemCoreClock == 0U) {
        return 0;
    }

    uint32_t period = (mode == PWM_CENTER) ? (2UL * res) : res;
    uint64_t requested_clock = (uint64_t)freq * period;
    uint32_t target = (requested_clock > UINT32_MAX) ? UINT32_MAX :
                      (uint32_t)requested_clock;
    uint32_t divider;
    uint32_t timer_clock = _select_divider(target, &divider);

    /* RIOT requires the resolution to be preserved and the frequency to be
     * lowered when the requested pair is not exactly representable. */
    if ((timer_clock == 0U) || ((timer_clock / period) == 0U)) {
        return 0;
    }

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

    timer->CAPCOM[0].VAL = (mode == PWM_CENTER) ? res : (res - 1U);
    /* Keep a complete CTRL image.  Do not use read-modify-write on this
     * hardware control register: not all written fields are guaranteed to be
     * reflected by a subsequent read. */
    uint32_t ctrl = (divider << TMR_CTRL_DIV_Pos) |
                    (TMR_CTRL_CLKSEL_SysClk << TMR_CTRL_CLKSEL_Pos);
    timer->CTRL = ctrl | TMR_CTRL_CLR_Msk;
    timer->CTRL = ctrl;

    for (unsigned i = 0; i < pwm_config[pwm].channel_numof; ++i) {
        _gpio_init(&pwm_config[pwm].channels[i]);
    }

    _state[pwm].resolution = res;
    _state[pwm].mode = mode;
    _state[pwm].ctrl = ctrl;

    uint32_t count_mode = (mode == PWM_CENTER) ? TMR_CTRL_MODE_UpDown :
                                                 TMR_CTRL_MODE_Up;
    timer->CTRL = ctrl | (count_mode << TMR_CTRL_MODE_Pos);

    return timer_clock / period;
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

    if ((value == 0U) || (value == resolution)) {
        capcom->CTRL = (value != 0U) ? TMR_CAPCOM_CTRL_OUT_Msk : 0U;
        return;
    }

    if (_state[pwm].mode == PWM_RIGHT) {
        capcom->VAL = (resolution - 1U) - value;
        _set_outmode(capcom, TMR_CAPCOM_CTRL_OUTMODE_Set_Reset);
    }
    else if (_state[pwm].mode == PWM_CENTER) {
        capcom->VAL = resolution - value;
        _set_outmode(capcom, TMR_CAPCOM_CTRL_OUTMODE_Toggle_Set);
    }
    else {
        capcom->VAL = value - 1U;
        _set_outmode(capcom, TMR_CAPCOM_CTRL_OUTMODE_Reset_Set);
    }
}

void pwm_poweron(pwm_t pwm)
{
    assert(pwm < PWM_NUMOF);
    RCU->CGCFGAPB |= pwm_config[pwm].clock_mask;
    TMR_TypeDef *timer = _dev(pwm);
    uint32_t mode = (_state[pwm].mode == PWM_CENTER) ?
                    TMR_CTRL_MODE_UpDown : TMR_CTRL_MODE_Up;

    timer->CTRL = _state[pwm].ctrl | (mode << TMR_CTRL_MODE_Pos);
}

void pwm_poweroff(pwm_t pwm)
{
    assert(pwm < PWM_NUMOF);
    _dev(pwm)->CTRL = _state[pwm].ctrl;
    RCU->CGCFGAPB &= ~pwm_config[pwm].clock_mask;
}
