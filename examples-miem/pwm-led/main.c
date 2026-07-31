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
 * @brief       Basic RIOT PWM API test on an LED
 *
 * @}
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "periph/pwm.h"
#include "ztimer.h"

/* PWM_DEV(1), channel 0 is TMR1_OUT3 on PA9. */
#ifndef PWM_DEV_NUM
#define PWM_DEV_NUM             (1U)
#endif

#ifndef PWM_CHANNEL
#define PWM_CHANNEL             (0U)
#endif

#ifndef PWM_MODE
#define PWM_MODE                PWM_LEFT
#endif

#ifndef PWM_FREQUENCY
#define PWM_FREQUENCY     (800U)
#endif

#ifndef PWM_RESOLUTION
#define PWM_RESOLUTION    (1250U)
#endif

#define PWM_DEVICE              PWM_DEV(PWM_DEV_NUM)
#define LEVEL_DELAY_MS          (5000U)

_Static_assert(PWM_FREQUENCY > 0U, "PWM frequency must not be zero");
_Static_assert(PWM_RESOLUTION >= 2U, "PWM resolution is too small");
_Static_assert(PWM_RESOLUTION <= UINT16_MAX, "PWM resolution is too large");

static const uint16_t duty_values[] = {
    0U,
    1U,
    PWM_RESOLUTION / 4U,
    PWM_RESOLUTION / 2U,
    (3U * PWM_RESOLUTION) / 4U,
    PWM_RESOLUTION - 1U,
    PWM_RESOLUTION,
};

static const char *_mode_name(pwm_mode_t mode)
{
    switch (mode) {
        case PWM_LEFT:
            return "left";
        case PWM_RIGHT:
            return "right";
        case PWM_CENTER:
            return "center";
        default:
            return "invalid";
    }
}

int main(void)
{
    uint32_t actual_frequency = pwm_init(PWM_DEVICE, PWM_MODE,
                                         PWM_FREQUENCY, PWM_RESOLUTION);

    if (actual_frequency == 0U) {
        printf("PWM initialization failed: dev=%u mode=%s freq=%" PRIu32
               " Hz res=%u\n", PWM_DEV_NUM, _mode_name(PWM_MODE),
               (uint32_t)PWM_FREQUENCY, PWM_RESOLUTION);
        return 1;
    }

    uint8_t channels = pwm_channels(PWM_DEVICE);
    if (PWM_CHANNEL >= channels) {
        printf("Invalid PWM channel %u, device has %u channel(s)\n",
               PWM_CHANNEL, channels);
        return 1;
    }

    printf("PWM initialized: dev=%u channel=%u mode=%s requested=%" PRIu32
           " Hz actual=%" PRIu32 " Hz resolution=%u\n",
           PWM_DEV_NUM, PWM_CHANNEL, _mode_name(PWM_MODE),
           (uint32_t)PWM_FREQUENCY, actual_frequency, PWM_RESOLUTION);

    while (1) {
        for (unsigned i = 0; i < sizeof(duty_values) / sizeof(duty_values[0]);
             ++i) {
            uint16_t value = duty_values[i];
            uint32_t percent = ((uint32_t)value * 100U) / PWM_RESOLUTION;

            pwm_set(PWM_DEVICE, PWM_CHANNEL, value);
            printf("pwm_set(value=%u): %" PRIu32 "%%\n", value, percent);
            ztimer_sleep(ZTIMER_MSEC, LEVEL_DELAY_MS);
        }
    }

    return 0;
}
