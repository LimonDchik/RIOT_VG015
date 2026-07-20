# PWM LED for BlueBird VG015

This example uses the standard RIOT PWM API to change the brightness of an LED
connected to `PA9`. The pin is routed to `TMR1_OUT3` and is exposed as
`PWM_DEV(1)`, channel 0 in the BlueBird VG015 peripheral configuration.

The brightness changes every three seconds in the following sequence:

```text
10% -> 50% -> 90% -> 50% -> 10% -> ...
```

The example calls `pwm_init()` and `pwm_set()` from RIOT's `periph_pwm` API.
The three-second delay is implemented with `ztimer_msec`.

The example assumes an active-low LED. If the LED is connected active-high,
set `LED_ACTIVE_LOW` to `0` in `main.c`.

Build with:

```sh
make -C examples-miem/pwm-led BOARD=bluebirdVG015
```

Flash with:

```sh
make -C examples-miem/pwm-led BOARD=bluebirdVG015 flash
```
