/*
 * Copyright (C) 2026
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu.h"
#include "irq.h"
#include "periph/timer.h"
#include "plic.h"

extern uint32_t SystemCoreClock;

void SystemCoreClockUpdate(void);

/**
 * @brief   Interrupt context for each configured timer
 */
static timer_isr_ctx_t isr_ctx[TIMER_NUMOF];

/**
 * @brief   Bitmap of one-shot channels
 */
static uint8_t _oneshot[(TIMER_NUMOF + 1) / 2];

/**
 * @brief   Bitmap of periodic channels
 */
static uint8_t _periodic[(TIMER_NUMOF + 1) / 2];

/**
 * @brief   Bitmap of channels that reset the counter on match
 */
static uint8_t _reset_on_match[(TIMER_NUMOF + 1) / 2];

/**
 * @brief   Stored periodic interval for each timer channel
 */
static uint32_t _period[TIMER_NUMOF][TIMER_CHANNEL_NUMOF];

/**
 * @brief   Helper macro to get channel bit in timer/channel bitmap
 */
#define CHAN_BIT(tim, chan) ((uint8_t)((1U << (chan)) << (TIMER_CHANNEL_NUMOF * ((tim) & 1U))))

static inline TMR32_TypeDef *dev(tim_t tim)
{
    return timer_config[tim].dev;
}

static inline volatile uint32_t *chan_reg(tim_t tim, unsigned chan)
{
    return &dev(tim)->CAPCOM[chan].VAL;
}

static inline void _set_oneshot(tim_t tim, unsigned chan)
{
    _oneshot[tim >> 1] |= CHAN_BIT(tim, chan);
}

static inline void _clear_oneshot(tim_t tim, unsigned chan)
{
    _oneshot[tim >> 1] &= (uint8_t)~CHAN_BIT(tim, chan);
}

static inline bool _is_oneshot(tim_t tim, unsigned chan)
{
    return (_oneshot[tim >> 1] & CHAN_BIT(tim, chan)) != 0;
}

static inline void _set_periodic(tim_t tim, unsigned chan)
{
    _periodic[tim >> 1] |= CHAN_BIT(tim, chan);
}

static inline void _clear_periodic(tim_t tim, unsigned chan)
{
    _periodic[tim >> 1] &= (uint8_t)~CHAN_BIT(tim, chan);
}

static inline bool _is_periodic(tim_t tim, unsigned chan)
{
    return (_periodic[tim >> 1] & CHAN_BIT(tim, chan)) != 0;
}

static inline void _set_reset_on_match(tim_t tim, unsigned chan)
{
    _reset_on_match[tim >> 1] |= CHAN_BIT(tim, chan);
}

static inline void _clear_reset_on_match(tim_t tim, unsigned chan)
{
    _reset_on_match[tim >> 1] &= (uint8_t)~CHAN_BIT(tim, chan);
}

static inline bool _is_reset_on_match(tim_t tim, unsigned chan)
{
    return (_reset_on_match[tim >> 1] & CHAN_BIT(tim, chan)) != 0;
}

static inline uint32_t _channel_im_mask(unsigned chan)
{
    return (uint32_t)(TMR32_IM_CAP0_Msk << chan);
}

static inline uint32_t _channel_mis_mask(unsigned chan)
{
    return (uint32_t)(TMR32_MIS_CAP0_Msk << chan);
}

static inline uint32_t _channel_ic_mask(unsigned chan)
{
    return (uint32_t)(TMR32_IC_CAP0_Msk << chan);
}

static int _pick_divider(uint32_t freq, uint32_t *div_sel, uint32_t *ticks_per_sec)
{
    SystemCoreClockUpdate();

    if ((freq == 0U) || (SystemCoreClock == 0U)) {
        return -1;
    }

    if (SystemCoreClock == freq) {
        *div_sel = TMR32_CTRL_DIV_Div1;
        *ticks_per_sec = SystemCoreClock;
        return 0;
    }
    if ((SystemCoreClock / 2U) == freq && ((SystemCoreClock % 2U) == 0U)) {
        *div_sel = TMR32_CTRL_DIV_Div2;
        *ticks_per_sec = SystemCoreClock / 2U;
        return 0;
    }
    if ((SystemCoreClock / 4U) == freq && ((SystemCoreClock % 4U) == 0U)) {
        *div_sel = TMR32_CTRL_DIV_Div4;
        *ticks_per_sec = SystemCoreClock / 4U;
        return 0;
    }
    if ((SystemCoreClock / 8U) == freq && ((SystemCoreClock % 8U) == 0U)) {
        *div_sel = TMR32_CTRL_DIV_Div8;
        *ticks_per_sec = SystemCoreClock / 8U;
        return 0;
    }

    return -1;
}

static int _find_reset_channel(tim_t tim)
{
    for (unsigned chan = 0; chan < TIMER_CHANNEL_NUMOF; chan++) {
        if (_is_periodic(tim, chan) && _is_reset_on_match(tim, chan)) {
            return (int)chan;
        }
    }

    return -1;
}

static void _timer_isr(int irq)
{
    tim_t tim = TIMER_DEV(0);
    TMR32_TypeDef *timer = dev(tim);
    uint32_t mis = timer->MIS;

    (void)irq;

    for (unsigned chan = 0; chan < TIMER_CHANNEL_NUMOF; chan++) {
        uint32_t mask = _channel_mis_mask(chan);

        if ((mis & mask) == 0U) {
            continue;
        }

        timer->IC = _channel_ic_mask(chan);

        if (_is_periodic(tim, chan)) {
            if (_is_reset_on_match(tim, chan)) {
                timer->COUNT = 0;
            }
            else {
                *chan_reg(tim, chan) += _period[tim][chan];
            }
        }
        else {
            timer->IM &= ~_channel_im_mask(chan);
        }

        if (isr_ctx[tim].cb != NULL) {
            isr_ctx[tim].cb(isr_ctx[tim].arg, (int)chan);
        }
    }
}

int timer_init(tim_t tim, uint32_t freq, timer_cb_t cb, void *arg)
{
    uint32_t div_sel;
    uint32_t ticks_per_sec;

    if (tim >= TIMER_NUMOF) {
        return -1;
    }

    if (_pick_divider(freq, &div_sel, &ticks_per_sec) != 0) {
        return -1;
    }

    (void)ticks_per_sec;

    isr_ctx[tim].cb = cb;
    isr_ctx[tim].arg = arg;

    _oneshot[tim >> 1] = 0;
    _periodic[tim >> 1] = 0;
    _reset_on_match[tim >> 1] = 0;
    for (unsigned chan = 0; chan < TIMER_CHANNEL_NUMOF; chan++) {
        _period[tim][chan] = 0;
    }

    RCU->CGCFGAPB_bit.TMR32EN = 1;
    RCU->RSTDISAPB_bit.TMR32EN = 1;

    dev(tim)->CTRL = 0;
    dev(tim)->COUNT = 0;
    dev(tim)->IM = 0;
    for (unsigned chan = 0; chan < TIMER_CHANNEL_NUMOF; chan++) {
        dev(tim)->CAPCOM[chan].CTRL = 0;
        dev(tim)->CAPCOM[chan].VAL = 0;
    }
    dev(tim)->IC = TMR32_IC_TMR_Msk |
                   TMR32_IC_CAP0_Msk | TMR32_IC_CAP1_Msk |
                   TMR32_IC_CAP2_Msk | TMR32_IC_CAP3_Msk;
    dev(tim)->CTRL_bit.CLR = 1;
    dev(tim)->CTRL_bit.CLR = 0;
    dev(tim)->CTRL_bit.DIV = div_sel;
    dev(tim)->CTRL_bit.CLKSEL = TMR32_CTRL_CLKSEL_SysClk;

    plic_set_isr_cb(timer_config[tim].irqn, _timer_isr);
    plic_set_priority(timer_config[tim].irqn, TMR_INTR_PRIORITY);
    plic_enable_interrupt(timer_config[tim].irqn);

    timer_start(tim);
    return 0;
}

int timer_set_absolute(tim_t tim, int channel, unsigned int value)
{
    if ((tim >= TIMER_NUMOF) || ((unsigned)channel >= TIMER_CHANNEL_NUMOF)) {
        return -1;
    }

    unsigned irqstate = irq_disable();

    _set_oneshot(tim, (unsigned)channel);
    _clear_periodic(tim, (unsigned)channel);
    _clear_reset_on_match(tim, (unsigned)channel);
    _period[tim][channel] = 0;

    dev(tim)->IC = _channel_ic_mask((unsigned)channel);
    *chan_reg(tim, (unsigned)channel) = value;
    dev(tim)->IM |= _channel_im_mask((unsigned)channel);

    irq_restore(irqstate);
    return 0;
}

int timer_set(tim_t tim, int channel, unsigned int timeout)
{
    uint32_t now;
    uint32_t value;
    uint32_t delta;
    unsigned irqstate;

    if ((tim >= TIMER_NUMOF) || ((unsigned)channel >= TIMER_CHANNEL_NUMOF)) {
        return -1;
    }

    irqstate = irq_disable();

    now = dev(tim)->COUNT;
    value = now + timeout;

    _set_oneshot(tim, (unsigned)channel);
    _clear_periodic(tim, (unsigned)channel);
    _clear_reset_on_match(tim, (unsigned)channel);
    _period[tim][channel] = 0;

    dev(tim)->IC = _channel_ic_mask((unsigned)channel);
    *chan_reg(tim, (unsigned)channel) = value;
    dev(tim)->IM |= _channel_im_mask((unsigned)channel);

    delta = (*chan_reg(tim, (unsigned)channel) - dev(tim)->COUNT);
    if (delta > timeout) {
        *chan_reg(tim, (unsigned)channel) = dev(tim)->COUNT + 1U;
    }
    if (timeout == 0U) {
        *chan_reg(tim, (unsigned)channel) = dev(tim)->COUNT + 1U;
    }

    irq_restore(irqstate);
    return 0;
}

int timer_set_periodic(tim_t tim, int channel, unsigned int value, uint8_t flags)
{
    unsigned irqstate;
    int reset_chan;

    if ((tim >= TIMER_NUMOF) || ((unsigned)channel >= TIMER_CHANNEL_NUMOF)) {
        return -1;
    }

    irqstate = irq_disable();

    if (flags & TIM_FLAG_RESET_ON_MATCH) {
        reset_chan = _find_reset_channel(tim);
        if ((reset_chan >= 0) && (reset_chan != channel)) {
            irq_restore(irqstate);
            return -1;
        }
    }

    if (flags & TIM_FLAG_SET_STOPPED) {
        timer_stop(tim);
    }

    if (flags & TIM_FLAG_RESET_ON_SET) {
        dev(tim)->COUNT = 0;
    }

    _clear_oneshot(tim, (unsigned)channel);
    _set_periodic(tim, (unsigned)channel);
    _period[tim][channel] = value;

    if (flags & TIM_FLAG_RESET_ON_MATCH) {
        _set_reset_on_match(tim, (unsigned)channel);
    }
    else {
        _clear_reset_on_match(tim, (unsigned)channel);
    }

    dev(tim)->IC = _channel_ic_mask((unsigned)channel);
    *chan_reg(tim, (unsigned)channel) = value;
    dev(tim)->IM |= _channel_im_mask((unsigned)channel);

    if ((flags & TIM_FLAG_SET_STOPPED) == 0U) {
        timer_start(tim);
    }

    irq_restore(irqstate);
    return 0;
}

int timer_clear(tim_t tim, int channel)
{
    unsigned irqstate;

    if ((tim >= TIMER_NUMOF) || ((unsigned)channel >= TIMER_CHANNEL_NUMOF)) {
        return -1;
    }

    irqstate = irq_disable();

    dev(tim)->IM &= ~_channel_im_mask((unsigned)channel);
    dev(tim)->IC = _channel_ic_mask((unsigned)channel);
    _clear_oneshot(tim, (unsigned)channel);
    _clear_periodic(tim, (unsigned)channel);
    _clear_reset_on_match(tim, (unsigned)channel);
    _period[tim][channel] = 0;

    irq_restore(irqstate);
    return 0;
}

unsigned int timer_read(tim_t tim)
{
    if (tim >= TIMER_NUMOF) {
        return 0;
    }

    return dev(tim)->COUNT;
}

void timer_start(tim_t tim)
{
    if (tim >= TIMER_NUMOF) {
        return;
    }

    dev(tim)->CTRL_bit.MODE = TMR32_CTRL_MODE_Multiple;
}

void timer_stop(tim_t tim)
{
    if (tim >= TIMER_NUMOF) {
        return;
    }

    dev(tim)->CTRL_bit.MODE = TMR32_CTRL_MODE_Stop;
}
