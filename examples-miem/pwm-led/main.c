/*
 * Copyright (C) 2026
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       LED brightness control using the RIOT PWM API
 *
 * @}
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "periph/pwm.h"
#include "ztimer.h"

/* PWM_DEV(1), channel 0 is TMR1_OUT3 on PA9. */
#define LED_PWM_DEV            PWM_DEV(1)
#define LED_PWM_CHANNEL        (0U)
#define PWM_FREQUENCY          (1000U)
#define PWM_RESOLUTION         (1000U)
#define LEVEL_DELAY_MS         (3000U)

/* BlueBird LEDs are connected active-low. Set to 0 for an active-high LED. */
#define LED_ACTIVE_LOW         (1U)

static const uint8_t brightness_levels[] = { 10, 50, 90, 50 };

static void _set_brightness(uint8_t percent)
{
    uint16_t value = ((uint32_t)PWM_RESOLUTION * percent) / 100U;

#if LED_ACTIVE_LOW
    value = PWM_RESOLUTION - value;
#endif

    pwm_set(LED_PWM_DEV, LED_PWM_CHANNEL, value);
}

int main(void)
{
    uint32_t actual_frequency = pwm_init(LED_PWM_DEV, PWM_LEFT,
                                         PWM_FREQUENCY, PWM_RESOLUTION);

    if (actual_frequency == 0U) {
        puts("Failed to initialize PWM on PA9");
        return 1;
    }

    printf("PWM on PA9 initialized at %" PRIu32 " Hz\n", actual_frequency);

    while (1) {
        for (unsigned i = 0;
             i < sizeof(brightness_levels) / sizeof(brightness_levels[0]);
             ++i) {
            uint8_t brightness = brightness_levels[i];

            _set_brightness(brightness);
            printf("LED brightness: %u%%\n", brightness);
            ztimer_sleep(ZTIMER_MSEC, LEVEL_DELAY_MS);
        }
    }

    return 0;
}
