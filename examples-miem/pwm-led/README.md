# PWM LED for BlueBird VG015

This example is a small test for the standard RIOT `periph_pwm` API. By
default, it drives the LED connected to `PA9`. The pin is routed to
`TMR1_OUT3` and is exposed as `PWM_DEV(1)`, channel 0.

The application passes frequency and resolution independently to `pwm_init()`
and prints both the requested and returned frequencies. It then calls
`pwm_set()` with values that exercise the complete RIOT duty-cycle range:

```text
0, 1, 25%, 50%, 75%, resolution - 1, resolution
```

Each value is held for five seconds before the application advances to the
next one.

The BlueBird LED is active-low, so its visible brightness is inverted. The raw
PWM values are intentionally not inverted because this application tests the
driver API rather than LED brightness semantics.

The following definitions at the top of `main.c` select the test parameters:

```c
#define PWM_DEV_NUM       (1U)
#define PWM_CHANNEL       (0U)
#define PWM_MODE          PWM_LEFT
#define PWM_FREQUENCY     (1000U)
#define PWM_RESOLUTION    (1000U)
```

`PWM_MODE` can be set to `PWM_LEFT`, `PWM_RIGHT`, or `PWM_CENTER`. To observe
left/right alignment, use an oscilloscope and compare the output with another
channel of the same PWM device; brightness alone cannot show pulse alignment.

If the requested frequency and resolution cannot be represented while keeping
the resolution unchanged, a conforming driver returns `0` and the application
prints `PWM initialization failed`.

Useful exact test pairs for the 1 MHz timer clock are:

| Frequency | Resolution |
|----------:|-----------:|
| 1000 Hz   | 1000       |
| 500 Hz    | 1000       |
| 250 Hz    | 1000       |
| 125 Hz    | 1000       |
| 100 Hz    | 10000      |

Build with:

```sh
make -C examples-miem/pwm-led BOARD=bluebirdVG015
```

Flash with:

```sh
make -C examples-miem/pwm-led BOARD=bluebirdVG015 flash
```
