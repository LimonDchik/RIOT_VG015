#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "K1921VG015.h"

/* LED on HL1 (GPIO_B10) */
#define BLUEBIRD_LED_GPIO      GPIOC
#define BLUEBIRD_LED_PIN_NUM   0
#define BLUEBIRD_LED_ACTIVE_LOW 1

#define LED0_PIN    (1U << BLUEBIRD_LED_PIN_NUM)
#ifdef BLUEBIRD_LED_ACTIVE_LOW
#define LED0_ON     (BLUEBIRD_LED_GPIO->DATAOUTCLR = LED0_PIN)
#define LED0_OFF    (BLUEBIRD_LED_GPIO->DATAOUTSET = LED0_PIN)
#define LED0_TOGGLE (BLUEBIRD_LED_GPIO->DATAOUTTGL = LED0_PIN)
#else
#define LED0_ON     (BLUEBIRD_LED_GPIO->DATAOUTSET = LED0_PIN)
#define LED0_OFF    (BLUEBIRD_LED_GPIO->DATAOUTCLR = LED0_PIN)
#define LED0_TOGGLE (BLUEBIRD_LED_GPIO->DATAOUTTGL = LED0_PIN)
#endif

void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
