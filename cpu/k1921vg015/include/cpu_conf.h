#ifndef CPU_CONF_H
#define CPU_CONF_H

#include "cpu_conf_common.h"
#include "vendor/clint.h"
#include "vendor/plic.h"

#ifdef __cplusplus
extern "C" {
#endif


#define PLIC_CTRL_ADDR        (0x0C000000UL)
#define PLIC_NUM_INTERRUPTS   (31)
#define PLIC_NUM_PRIORITIES   (7)

/** @} */

/**
 * @name CLINT / machine timer (mtime / mtimecmp)
 *
 * Same layout as SiFive-style CLINT (`cpu/riscv_common/include/vendor/clint.h`).
 * Matches `RISCV_MTIME_ADDR` / `RISCV_MTIMECMP_ADDR` in `mtimer.h`.
 * @{
 */
#ifndef CLINT_BASE_ADDR
#define CLINT_BASE_ADDR         (0x02000000UL)
#endif

/**
 * @brief Base address of the PLIC peripheral
 */
#define PLIC_BASE_ADDR      (PLIC_CTRL_ADDR)

/** @} */

/**
 * @brief   TMR interrupt priority
 */
#define TMR_INTR_PRIORITY  (1)

#ifdef __cplusplus
}
#endif

#endif /* CPU_CONF_H */
