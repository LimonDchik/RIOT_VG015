/*
 * Copyright (C) 2025
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "periph/uart.h"
#include "periph_conf.h"

extern uint32_t SystemCoreClock;

void SystemCoreClockUpdate(void);

static void _uart_hw_init(UART_TypeDef *uart, uint32_t baud)
{
    uint32_t clk = SystemCoreClock;

    uint32_t baud_icoef = clk / (16 * baud);
    uint32_t baud_fcoef =
        (uint32_t)(((clk / (16.0f * baud) - baud_icoef) * 64.0f) + 0.5f);

    /* Same bring-up as vendor retarget.c: UART0 on GPIOA (TX pin1, RX pin0). */
    RCU->CGCFGAHB_bit.GPIOAEN = 1;
    RCU->RSTDISAHB_bit.GPIOAEN = 1;
    RCU->CGCFGAPB_bit.UART0EN = 1;
    RCU->RSTDISAPB_bit.UART0EN = 1;

    GPIOA->ALTFUNCNUM_bit.PIN0 = 1;
    GPIOA->ALTFUNCNUM_bit.PIN1 = 1;
    GPIOA->ALTFUNCSET = (1U << 0) | (1U << 1);

    RCU->UARTCLKCFG[0].UARTCLKCFG =
        (1U << RCU_UARTCLKCFG_CLKSEL_Pos) | RCU_UARTCLKCFG_CLKEN_Msk |
        RCU_UARTCLKCFG_RSTDIS_Msk;

    uart->IBRD = baud_icoef;
    uart->FBRD = baud_fcoef;
    uart->LCRH = UART_LCRH_FEN_Msk | (3U << UART_LCRH_WLEN_Pos);
    uart->CR = UART_CR_TXE_Msk | UART_CR_RXE_Msk | UART_CR_UARTEN_Msk;
}

int uart_init(uart_t dev, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    (void)arg;

    assert(dev < UART_NUMOF);

    if (rx_cb != NULL) {
        return -ENOTSUP;
    }

    SystemCoreClockUpdate();
    _uart_hw_init(uart_config[dev].dev, baudrate);

    return 0;
}

void uart_write(uart_t dev, const uint8_t *data, size_t len)
{
    UART_TypeDef *uart = uart_config[dev].dev;

    for (size_t i = 0; i < len; i++) {
        uint32_t spin = 0;
        while (uart->FR_bit.TXFF) {
            /* Avoid hard-lock if UART TX FIFO status never clears. */
            if (++spin > 1000000u) {
                break;
            }
        }
        uart->DR = data[i];
    }
}

void uart_poweron(uart_t dev)
{
    (void)dev;
}

void uart_poweroff(uart_t dev)
{
    (void)dev;
}

int uart_mode(uart_t uart, uart_data_bits_t data_bits, uart_parity_t parity,
              uart_stop_bits_t stop_bits)
{
    (void)uart;
    if (data_bits == UART_DATA_BITS_8 && parity == UART_PARITY_NONE &&
        stop_bits == UART_STOP_BITS_1) {
        return 0;
    }
    return -ENOTSUP;
}
