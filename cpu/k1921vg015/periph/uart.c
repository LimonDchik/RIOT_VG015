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

#include "cpu.h"
#include "periph/uart.h"
#include "periph_conf.h"
#include "plic.h"

extern uint32_t SystemCoreClock;

void SystemCoreClockUpdate(void);

#ifndef BLUEBIRD_UART_HSECLK
#define BLUEBIRD_UART_HSECLK (16000000U)
#endif

static uart_rx_cb_t _rx_cb[UART_NUMOF];
static void *_rx_arg[UART_NUMOF];

static unsigned _uart_vector_num(UART_TypeDef *uart)
{
    if (uart == UART0) {
        return PLIC_UART0_VECTNUM;
    }
    if (uart == UART1) {
        return PLIC_UART1_VECTNUM;
    }
    if (uart == UART2) {
        return PLIC_UART2_VECTNUM;
    }
    if (uart == UART3) {
        return PLIC_UART3_VECTNUM;
    }
    if (uart == UART4) {
        return PLIC_UART4_VECTNUM;
    }

    return 0;
}

static void _uart_config_pins(UART_TypeDef *uart)
{
    RCU->CGCFGAHB_bit.GPIOAEN = 1;
    RCU->RSTDISAHB_bit.GPIOAEN = 1;

    if (uart == UART3) {
        /* BlueBird-VG015 USR2UART bridge is wired to UART3 on PA14/PA15. */
        RCU->CGCFGAPB_bit.UART3EN = 1;
        RCU->RSTDISAPB_bit.UART3EN = 1;

        GPIOA->ALTFUNCNUM_bit.PIN14 = 3;
        GPIOA->ALTFUNCNUM_bit.PIN15 = 3;
        GPIOA->ALTFUNCSET = GPIO_ALTFUNCSET_PIN14_Msk |
                            GPIO_ALTFUNCSET_PIN15_Msk;

        RCU->UARTCLKCFG[3].UARTCLKCFG =
            (RCU_UARTCLKCFG_CLKSEL_HSE << RCU_UARTCLKCFG_CLKSEL_Pos) |
            RCU_UARTCLKCFG_CLKEN_Msk | RCU_UARTCLKCFG_RSTDIS_Msk;
        return;
    }

    if (uart == UART0) {
        RCU->CGCFGAPB_bit.UART0EN = 1;
        RCU->RSTDISAPB_bit.UART0EN = 1;

        GPIOA->ALTFUNCNUM_bit.PIN0 = 1;
        GPIOA->ALTFUNCNUM_bit.PIN1 = 1;
        GPIOA->ALTFUNCSET = (1U << 0) | (1U << 1);

        RCU->UARTCLKCFG[0].UARTCLKCFG =
            (1U << RCU_UARTCLKCFG_CLKSEL_Pos) | RCU_UARTCLKCFG_CLKEN_Msk |
            RCU_UARTCLKCFG_RSTDIS_Msk;
    }
}

static void _uart_isr(uart_t dev)
{
    UART_TypeDef *uart = uart_config[dev].dev;

    while (!uart->FR_bit.RXFE) {
        _rx_cb[dev](_rx_arg[dev], (uint8_t)uart->DR_bit.DATA);
    }

    uart->ICR = UART_ICR_RXIC_Msk | UART_ICR_RTIC_Msk |
                UART_ICR_FEIC_Msk | UART_ICR_PEIC_Msk |
                UART_ICR_BEIC_Msk | UART_ICR_OEIC_Msk;
}

static void _uart0_isr(void) { _uart_isr(0); }

static uint32_t _uart_clk(UART_TypeDef *uart)
{
    if (uart == UART3) {
        return BLUEBIRD_UART_HSECLK;
    }

    return SystemCoreClock;
}

static uint32_t _uart_lcrh(UART_TypeDef *uart)
{
    /* BlueBird's working UART3 test uses 8N1 without FIFO. */
    if (uart == UART3) {
        return (3U << UART_LCRH_WLEN_Pos);
    }

    return UART_LCRH_FEN_Msk | (3U << UART_LCRH_WLEN_Pos);
}

static void _uart_hw_init(UART_TypeDef *uart, uint32_t baud)
{
    uint32_t clk = _uart_clk(uart);
    uint32_t divisor_x64 = ((clk * 4U) + (baud / 2U)) / baud;
    uint32_t baud_icoef = divisor_x64 / 64U;
    uint32_t baud_fcoef = divisor_x64 - (baud_icoef * 64U);

    _uart_config_pins(uart);

    uart->CR = 0;
    uart->IBRD = baud_icoef;
    uart->FBRD = baud_fcoef;
    uart->LCRH = _uart_lcrh(uart);
    uart->IFLS = 0;
    uart->ICR = 0x7ffU;
    uart->CR = UART_CR_TXE_Msk | UART_CR_RXE_Msk | UART_CR_UARTEN_Msk;
}

int uart_init(uart_t dev, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    assert(dev < UART_NUMOF);

    if (_uart_vector_num(uart_config[dev].dev) == 0) {
        return -ENOTSUP;
    }

    SystemCoreClockUpdate();
    _uart_hw_init(uart_config[dev].dev, baudrate);

    _rx_cb[dev] = rx_cb;
    _rx_arg[dev] = arg;

    if (rx_cb != NULL) {
        uart_config[dev].dev->ICR = 0x7ffU;
        uart_config[dev].dev->IMSC = UART_IMSC_RXIM_Msk;
        SetIrqHandler(_uart_vector_num(uart_config[dev].dev), _uart0_isr, 1);
    }
    else {
        uart_config[dev].dev->IMSC = 0;
        PLIC_IntDisable(Plic_Mach_Target, _uart_vector_num(uart_config[dev].dev));
    }

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
